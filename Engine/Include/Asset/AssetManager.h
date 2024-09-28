#pragma once
#include "Asset/AssetRegister.h"
#include "Asset/IResource.h"
#include "Asset/Texture.h"
#include "Asset/TextureResource.h"
#include "Umbra.h"
namespace Umbra {
    class AssetManager : public Singleton<AssetManager> {
    private:
        /* data */
    public:
        AssetManager();
        SharedPtr<Texture> GetTexture(const String& _filePath);

    private:
        template <typename T>
        SharedPtr<T> GetResource(const String& _filePath);
        // AddResourceRef
        // FetchResource
        // GetResource

        template <typename T>
        void UnloadResource();

    private:
        UniquePtr<AssetRegister> assetRegister;
    };

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
