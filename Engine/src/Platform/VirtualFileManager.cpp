#include "Platform/VirtualFileManager.h"

bool Umbra::Platform::VirtualFileManager::Mount(const String& _virtualPath, const String& _mountPath,
    SharedPtr<IVirtualFileSystem> _fileSystemProvider, int _priority) {
    if (_fileSystemProvider == nullptr) {
        UMBRA_LOG_ERROR("VirtualFileManager Invalid IVirtualFileSystem provider : Null at %s", _mountPath.c_str());
        return false;
    }
    // todo @pavan [normalize case to prevent errors]

    // check if already mounted
    for (MountPoint& mountPoint : mMountPoints) {
        if (mountPoint.mVirtualPath.compare(_virtualPath) == 0) {
            if (mountPoint.mMountPath.compare(_mountPath) == 0) {
                return true;
            } else if (mountPoint.mPriority == _priority) {
                UMBRA_LOG_WARNING("VirtualFileManager Attempting to mount to an already mounted path: %s", _mountPath.c_str());
            }
        }
    }

    // add new mount point and
    MountPoint mount;
    mount.mFS          = std::move(_fileSystemProvider);
    mount.mVirtualPath = _virtualPath;
    mount.mMountPath   = _mountPath;
    mMountPoints.push_back(std::move(mount));

    // sort mount points to take priority in consideration and optimize to look deep paths first
    std::sort(mMountPoints.begin(), mMountPoints.end(), [](const MountPoint& _a, const MountPoint& _b) {
        if (_a.mPriority != _b.mPriority) {
            return _a.mPriority > _b.mPriority;
        }
        return _a.mVirtualPath.length() > _b.mVirtualPath.length();
    });

    return true;
}

namespace Umbra::Platform {

    String VirtualFileManager::ResolveRealPath(const String& _filePath, size_t& _outMountIndex) {
        for (size_t i = 0; i < mMountPoints.size(); i++) {
            if (_filePath.find(mMountPoints[i].mVirtualPath) == 0) {
                String realPath = mMountPoints[i].mMountPath + _filePath.substr(mMountPoints[i].mVirtualPath.size());
                if (!realPath.empty() && realPath[0] == '/') {
                    realPath = realPath.substr(1);
                }
                _outMountIndex = i;
                return realPath;
            }
        }
        return "";
    }

    UniquePtr<IFile> VirtualFileManager::Open(const String& _filePath, EFileMode _fileMode) {
        size_t mountIndex = 0;
        String realPath = ResolveRealPath(_filePath, mountIndex);
        if (!realPath.empty()) {
            return mMountPoints[mountIndex].mFS->OpenFile(realPath, _fileMode);
        }
        return nullptr;
    }

    bool VirtualFileManager::Exists(const String& _filePath) {
        size_t mountIndex = 0;
        String realPath = ResolveRealPath(_filePath, mountIndex);
        if (!realPath.empty()) {
            return mMountPoints[mountIndex].mFS->Exists(realPath);
        }
        return false;
    }

} // namespace Umbra::Platform
