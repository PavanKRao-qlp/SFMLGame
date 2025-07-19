#pragma once
#include "Asset/AssetRegister.h"
#include "Asset/IResource.h"
#include "Asset/Texture.h"
#include "Asset/TextureResource.h"
#include "Umbra.h"
namespace Umbra {

    /*

    */
    class AssetManager : public Singleton<AssetManager> {
    private:
        /* data */
    public:
        template <typename T>
        SharedPtr<T> Load(const String& _filePath);

        SharedPtr<Texture> GetTexture(const String& _filePath);

        template <typename T>
        SharedPtr<T> GetResource(const String& _filePath);

    private:
        AssetManager();
        // AddResourceRef
        // FetchResource
        // GetResource

        template <typename T>
        void UnloadResource();

    private:
        friend class Singleton<AssetManager>;
        UniquePtr<AssetRegister> assetRegister;
    };

    template <typename T>
    inline SharedPtr<T> AssetManager::Load(const String& _filePath) {

        return SharedPtr<T>();
    }

    template <typename T>
    SharedPtr<T> AssetManager::GetResource(const String& _filePath) {
        SharedPtr<T> res = nullptr;
        if (assetRegister.get()->GetResource(_filePath) == nullptr) {
            res                               = std::make_shared<T>(_filePath);
            SharedPtr<IResource>& newResource = std::static_pointer_cast<IResource>(res);
            assetRegister.get()->AddResource(_filePath, newResource);
            newResource->Load();
        } else {
            res = std::static_pointer_cast<T>(assetRegister.get()->GetResource(_filePath));
        }
        return res;
    }


} // namespace Umbra
