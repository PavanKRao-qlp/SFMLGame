#pragma once
#include "Platform/VirtualFileSystem.h"
#include "Umbra.h"

namespace Umbra::Platform {

    class NativeFileSystem : public IVirtualFileSystem {
    public:
        virtual UniquePtr<IFile> OpenFile(const String& _path, EFileMode _mode) override;
        virtual bool Exists(const String& _path) override;
    };

} // namespace Umbra::Platform
