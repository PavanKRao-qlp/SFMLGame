#pragma once
#include "Umbra.h"
namespace Umbra {

    class IResource {
    public:
        IResource(const String& _filePath) : filePath(_filePath) {}
        virtual ~IResource()  = default;
        virtual bool Load()   = 0;
        virtual void Unload() = 0;

        bool bLoaded = false;

    protected:
        const String& filePath;
    };

    // -------------------------------------------------------------------------
    // IResourceHandle<T>
    //
    // Base class for all user-facing asset handles (Texture, Audio, Font, ...).
    // Each handle holds a shared reference to its underlying resource, keeping
    // it alive in the AssetRegister's cache.
    //
    //   IsValid()   — handle is non-null (resource object exists)
    //   IsLoaded()  — underlying resource finished Load() successfully
    // -------------------------------------------------------------------------
    template<typename T>
    class IResourceHandle {
    public:
        bool IsValid()  const { return mResource != nullptr; }
        bool IsLoaded() const { return mResource && mResource->bLoaded; }

    protected:
        explicit IResourceHandle(SharedPtr<T> _resource) : mResource(std::move(_resource)) {}
        IResourceHandle() = default;

        SharedPtr<T> mResource;
    };

} // namespace Umbra
