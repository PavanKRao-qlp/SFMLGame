#pragma once
#include "Asset/AssetRegister.h"
#include "Asset/AudioResource.h"
#include "Asset/FontResource.h"
#include "Asset/IResource.h"
#include "Asset/ShaderResource.h"
#include "Asset/Texture.h"
#include "Asset/TextureResource.h"
#include "Thread/Mutex.h"
#include "Umbra.h"

namespace Umbra {

    class AssetManager : public Singleton<AssetManager> {
    public:
        // ---- Synchronous API ------------------------------------------------
        // Blocks until the resource is loaded (or returns it from cache).
        // Safe to call from the main thread.
        SharedPtr<Texture> GetTexture(const String& _filePath);
        SharedPtr<Audio>   GetAudio(const String& _filePath);
        SharedPtr<Font>    GetFont(const String& _filePath);

        // Fragment shader only (vertex pass-through).
        SharedPtr<Shader> GetShader(const String& _fragPath);
        // Vertex + fragment shader pair.
        SharedPtr<Shader> GetShader(const String& _vertPath, const String& _fragPath);

        // ---- Asynchronous API -----------------------------------------------
        // Returns immediately; the callback fires on a job-system worker thread
        // once the resource is loaded (or immediately if already cached).
        //
        // NOTE: The callback runs on a worker thread — do not touch SFML objects
        //       (sf::Sprite, sf::Sound, etc.) directly inside the callback.
        //       Use an AtomicBool or post back to the main thread instead.
        void GetTextureAsync(const String& _filePath, std::function<void(SharedPtr<Texture>)> _callback);
        void GetAudioAsync(const String& _filePath, std::function<void(SharedPtr<Audio>)> _callback);
        void GetFontAsync(const String& _filePath, std::function<void(SharedPtr<Font>)> _callback);
        void GetShaderAsync(const String& _fragPath, std::function<void(SharedPtr<Shader>)> _callback);
        void GetShaderAsync(const String& _vertPath, const String& _fragPath, std::function<void(SharedPtr<Shader>)> _callback);

    private:
        AssetManager();

        // ---- Internal helpers -----------------------------------------------

        // Synchronous resource getter — thread-safe. If the same path is being
        // loaded asynchronously, this call blocks until the async load finishes.
        template <typename T>
        SharedPtr<T> GetResource(const String& _filePath);

        // Core async implementation shared by GetTextureAsync / GetAudioAsync.
        //   _factory:    creates the unloaded IResource (called on the worker thread)
        //   _onComplete: called with the loaded IResource (on a worker thread, or inline if cached)
        void AsyncLoadImpl(const String&                              _filePath,
                           std::function<SharedPtr<IResource>()>      _factory,
                           std::function<void(SharedPtr<IResource>)>  _onComplete);

        template <typename T>
        void UnloadResource();

    private:
        friend class Singleton<AssetManager>;

        UniquePtr<AssetRegister> mAssetRegister;

        // Protects mAssetRegister and mPendingCallbacks
        Mutex mRegisterMutex;

        // Paths whose async load job is currently in-flight → waiting callbacks.
        // Key exists  → load in progress (callbacks fired when job completes).
        // Key absent  → not loading (or already in mAssetRegister).
        UMap<String, Vector<std::function<void(SharedPtr<IResource>)>>> mPendingCallbacks;
    };

    // -------------------------------------------------------------------------
    // GetResource<T> — synchronous, thread-safe
    // -------------------------------------------------------------------------
    template <typename T>
    SharedPtr<T> AssetManager::GetResource(const String& _filePath) {
        while (true) {
            bool doLoad = false;
            SharedPtr<T> newRes;

            {
                LockGuard<Mutex> lock(mRegisterMutex);

                // Cache hit
                SharedPtr<IResource> existing = mAssetRegister->GetResource(_filePath);
                if (existing) {
                    return std::static_pointer_cast<T>(existing);
                }

                // Not cached and not in-flight — claim the load
                if (mPendingCallbacks.count(_filePath) == 0) {
                    newRes = std::make_shared<T>(_filePath);
                    mPendingCallbacks[_filePath] = {};  // mark in-flight
                    doLoad = true;
                }
                // else: async load in-flight — fall through to wait below
            }

            if (doLoad) {
                // Load outside the lock so workers aren't blocked
                newRes->Load();

                // Register the result and collect any async callbacks that queued up
                Vector<std::function<void(SharedPtr<IResource>)>> callbacks;
                SharedPtr<IResource> iRes = std::static_pointer_cast<IResource>(newRes);
                {
                    LockGuard<Mutex> lock(mRegisterMutex);
                    mAssetRegister->AddResource(_filePath, iRes);
                    auto it = mPendingCallbacks.find(_filePath);
                    if (it != mPendingCallbacks.end()) {
                        callbacks = std::move(it->second);
                        mPendingCallbacks.erase(it);
                    }
                }

                // Fire callbacks outside the lock
                for (auto& cb : callbacks) {
                    cb(iRes);
                }
                return newRes;
            }

            // Another thread (sync or async) is loading this path — wait and retry
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

} // namespace Umbra
