#pragma once
#include "Platform/VirtualFileSystem.h"
#include "Umbra.h"

namespace Umbra::Platform {
    /// @brief Provides Abstract platform independent layer for file IO operations
    class VirtualFileManager {

        struct MountPoint {
            String mVirtualPath;
            String mMountPath;
            SharedPtr<IVirtualFileSystem> mFS;
            int mPriority;
        };

    public:
        /// @brief
        /// @param _virtualPath
        /// @param _mountPath
        /// @param _fileSystemProvider
        /// @param _priority
        /// @return
        bool Mount(const String& _virtualPath, const String& _mountPath,
            SharedPtr<IVirtualFileSystem> _fileSystemProvider, int _priority = 0);

        UniquePtr<IFile> Open(const String& _filePath, EFileMode _fileMode);
        bool Exists(const String& _filePath);

    private:
        String ResolveRealPath(const String& _filePath, size_t& _outMountIndex);

        Vector<MountPoint> mMountPoints;
    };
} // namespace Umbra::Platform
