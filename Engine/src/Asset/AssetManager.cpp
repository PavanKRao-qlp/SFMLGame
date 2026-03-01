#include "Asset/AssetManager.h"
#include "Asset/FontResource.h"
#include "Asset/ShaderResource.h"
#include "Diag/MemoryTracker.h"
#include "Service/ServiceLocator.h"
#include <chrono>

namespace Umbra {

    AssetManager::AssetManager() {
        mAssetRegister = std::make_unique<AssetRegister>();
    }

    // -------------------------------------------------------------------------
    // Synchronous API
    // -------------------------------------------------------------------------

    SharedPtr<Texture> AssetManager::GetTexture(const String& _filePath) {
        UMBRA_ALLOC_SCOPE(EMemoryCategory::Asset);
        SharedPtr<TextureResource> res = GetResource<TextureResource>(_filePath);
        if (res) {
            return std::make_shared<Texture>(res);
        }
        return nullptr;
    }

    SharedPtr<Audio> AssetManager::GetAudio(const String& _filePath) {
        UMBRA_ALLOC_SCOPE(EMemoryCategory::Asset);
        SharedPtr<AudioResource> res = GetResource<AudioResource>(_filePath);
        if (res) {
            return std::make_shared<Audio>(res);
        }
        return nullptr;
    }

    SharedPtr<Font> AssetManager::GetFont(const String& _filePath) {
        UMBRA_ALLOC_SCOPE(EMemoryCategory::Asset);
        SharedPtr<FontResource> res = GetResource<FontResource>(_filePath);
        if (res) {
            return std::make_shared<Font>(res);
        }
        return nullptr;
    }

    // -------------------------------------------------------------------------
    // Asynchronous API
    // -------------------------------------------------------------------------

    void AssetManager::GetTextureAsync(const String&                             _filePath,
                                       std::function<void(SharedPtr<Texture>)>   _callback) {
        AsyncLoadImpl(
            _filePath,
            // Factory — runs on worker thread
            [_filePath]() -> SharedPtr<IResource> {
                return std::make_shared<TextureResource>(_filePath);
            },
            // Completion — wraps the loaded resource into a Texture handle
            [cb = std::move(_callback)](SharedPtr<IResource> _res) {
                auto texRes = std::static_pointer_cast<TextureResource>(_res);
                cb(std::make_shared<Texture>(texRes));
            });
    }

    void AssetManager::GetAudioAsync(const String&                           _filePath,
                                     std::function<void(SharedPtr<Audio>)>   _callback) {
        AsyncLoadImpl(
            _filePath,
            // Factory — runs on worker thread
            [_filePath]() -> SharedPtr<IResource> {
                return std::make_shared<AudioResource>(_filePath);
            },
            // Completion — wraps the loaded resource into an Audio handle
            [cb = std::move(_callback)](SharedPtr<IResource> _res) {
                auto audioRes = std::static_pointer_cast<AudioResource>(_res);
                cb(std::make_shared<Audio>(audioRes));
            });
    }

    void AssetManager::GetFontAsync(const String&                          _filePath,
                                    std::function<void(SharedPtr<Font>)>   _callback) {
        AsyncLoadImpl(
            _filePath,
            // Factory — runs on worker thread
            [_filePath]() -> SharedPtr<IResource> {
                return std::make_shared<FontResource>(_filePath);
            },
            // Completion — wraps the loaded resource into a Font handle
            [cb = std::move(_callback)](SharedPtr<IResource> _res) {
                auto fontRes = std::static_pointer_cast<FontResource>(_res);
                cb(std::make_shared<Font>(fontRes));
            });
    }

    // -------------------------------------------------------------------------
    // Shader synchronous
    // -------------------------------------------------------------------------

    SharedPtr<Shader> AssetManager::GetShader(const String& _fragPath) {
        UMBRA_ALLOC_SCOPE(EMemoryCategory::Asset);
        SharedPtr<ShaderResource> res = GetResource<ShaderResource>(_fragPath);
        if (res) {
            return std::make_shared<Shader>(res);
        }
        return nullptr;
    }

    SharedPtr<Shader> AssetManager::GetShader(const String& _vertPath, const String& _fragPath) {
        UMBRA_ALLOC_SCOPE(EMemoryCategory::Asset);
        String combinedKey = _vertPath + "|" + _fragPath;
        SharedPtr<ShaderResource> res = GetResource<ShaderResource>(combinedKey);
        if (res) {
            return std::make_shared<Shader>(res);
        }
        return nullptr;
    }

    // -------------------------------------------------------------------------
    // Shader asynchronous
    // -------------------------------------------------------------------------

    void AssetManager::GetShaderAsync(const String&                           _fragPath,
                                      std::function<void(SharedPtr<Shader>)>  _callback) {
        AsyncLoadImpl(
            _fragPath,
            [_fragPath]() -> SharedPtr<IResource> {
                return std::make_shared<ShaderResource>(_fragPath);
            },
            [cb = std::move(_callback)](SharedPtr<IResource> _res) {
                auto shaderRes = std::static_pointer_cast<ShaderResource>(_res);
                cb(std::make_shared<Shader>(shaderRes));
            });
    }

    void AssetManager::GetShaderAsync(const String&                           _vertPath,
                                      const String&                           _fragPath,
                                      std::function<void(SharedPtr<Shader>)>  _callback) {
        String combinedKey = _vertPath + "|" + _fragPath;
        AsyncLoadImpl(
            combinedKey,
            [combinedKey]() -> SharedPtr<IResource> {
                return std::make_shared<ShaderResource>(combinedKey);
            },
            [cb = std::move(_callback)](SharedPtr<IResource> _res) {
                auto shaderRes = std::static_pointer_cast<ShaderResource>(_res);
                cb(std::make_shared<Shader>(shaderRes));
            });
    }

    // -------------------------------------------------------------------------
    // AsyncLoadImpl — core implementation
    //
    // Thread-safety contract:
    //   mRegisterMutex guards both mAssetRegister and mPendingCallbacks.
    //   The lock is never held while doing I/O (Load()) or while firing callbacks.
    //
    // Scenarios handled:
    //   1. Already cached  → callback fires immediately on the calling thread
    //   2. Load in-flight  → callback is queued, fired by the worker on completion
    //   3. First request   → job is submitted; all queued callbacks fire on done
    // -------------------------------------------------------------------------
    void AssetManager::AsyncLoadImpl(const String&                              _filePath,
                                     std::function<SharedPtr<IResource>()>      _factory,
                                     std::function<void(SharedPtr<IResource>)>  _onComplete) {
        SharedPtr<IResource> cached;

        {
            LockGuard<Mutex> lock(mRegisterMutex);

            cached = mAssetRegister->GetResource(_filePath);
            if (!cached) {
                auto it = mPendingCallbacks.find(_filePath);
                if (it != mPendingCallbacks.end()) {
                    // Load in-flight — queue and return
                    it->second.push_back(std::move(_onComplete));
                    return;
                }
                // First request — mark in-flight with this callback
                mPendingCallbacks[_filePath].push_back(std::move(_onComplete));
            }
        }

        // Cache hit — fire immediately outside the lock
        if (cached) {
            _onComplete(cached);
            return;
        }

        // Build the job — captures path and factory by value/move
        auto loadJob = [this, path = _filePath, factory = std::move(_factory)]() {
            // Tag this worker thread's allocations as Asset for the duration of the load
            UMBRA_ALLOC_SCOPE(EMemoryCategory::Asset);
            SharedPtr<IResource> res = factory();
            res->Load();

            // Register and collect all waiting callbacks
            Vector<std::function<void(SharedPtr<IResource>)>> callbacks;
            {
                LockGuard<Mutex> lock(mRegisterMutex);
                mAssetRegister->AddResource(path, res);
                auto it = mPendingCallbacks.find(path);
                if (it != mPendingCallbacks.end()) {
                    callbacks = std::move(it->second);
                    mPendingCallbacks.erase(it);
                }
            }

            // Fire all callbacks outside the lock
            for (auto& cb : callbacks) {
                cb(res);
            }
        };

        // Submit to job system; fall back to inline execution if unavailable
        JobSystem* js = ServiceLocator::GetJobSystem();
        if (js) {
            js->Submit(std::move(loadJob));
        } else {
            loadJob();
        }
    }

} // namespace Umbra
