#include "Platform/NativeFileSystem.h"

#include "Platform/NativeFile.h"
#include <filesystem>

Umbra::UniquePtr<Umbra::Platform::IFile> Umbra::Platform::NativeFileSystem::OpenFile(
    const String& _path, EFileMode _mode) {
    return std::make_unique<Umbra::Platform::NativeFile>(_path, _mode);
}

bool Umbra::Platform::NativeFileSystem::Exists(const String& _path) {
    return std::filesystem::exists(_path);
}
