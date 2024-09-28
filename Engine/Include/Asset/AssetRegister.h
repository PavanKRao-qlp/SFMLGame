#pragma once
#include "Asset/IResource.h"
#include "Umbra.h"
namespace Umbra {
    class AssetRegister {
    private:
        /* data */
    private:
        UMap<String, SharedPtr<IResource>> AssetRegisterMap;

    public:
        inline SharedPtr<IResource> GetResource(const String& _filePath) {
            if (AssetRegisterMap.find(_filePath) != AssetRegisterMap.end()) {
                return AssetRegisterMap[_filePath];
            }
            return nullptr;
        }
        inline bool AddResource(const String& _filePath, SharedPtr<IResource> _resource) {
            auto result = (AssetRegisterMap.insert_or_assign(_filePath, _resource));
            return result.second;
        }
        inline bool RemoveResource(const String& _filePath) {
            return AssetRegisterMap.erase(_filePath);
        }
    };
} // namespace Umbra
