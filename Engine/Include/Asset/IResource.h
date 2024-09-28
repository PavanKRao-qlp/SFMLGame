#pragma once
#include "Umbra.h"
namespace Umbra {
    class IResource {
    public:
        IResource(const String& _filePath) : filePath(_filePath) {};
        virtual ~IResource()  = default;
        virtual bool Load()   = 0;
        virtual void Unload() = 0;

    protected:
        const String& filePath;
    };
    class IResourceHandle {
    public:
        // virtual ~IResource()                       = default;
        // virtual bool Load(const String& _filePath) = 0;
        // virtual void Unload()                      = 0;
    };
} // namespace Umbra
