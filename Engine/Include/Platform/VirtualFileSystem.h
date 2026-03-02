#pragma once
#include "Umbra.h"

namespace Umbra::Platform {

    enum class EFileMode {
        Read,
        Write,
        ReadBinary,
        WriteBinary
    };

    /// @brief Interface to describe a File in a File System
    class IFile {
    public:
        IFile()          = default;
        virtual ~IFile() = default;
        // Prevent copying / moving(Rule of Five) IFile(const IFile&) = delete;
        IFile& operator=(const IFile&) = delete;
        IFile(IFile&&)                 = delete;
        IFile& operator=(IFile&&)      = delete;

        virtual uint64 Read(void* _buffer, uint64 _size)        = 0;
        virtual uint64 Write(const void* _buffer, uint64 _size) = 0;
        virtual bool Seek(uint64 _offset)                       = 0;
        virtual uint64 Tell()                                   = 0;
        virtual uint64 Size()                                   = 0;
        virtual bool Close()                                    = 0;
        virtual bool IsValid() const                            = 0;
    };

    /// @brief Core Virtual File interface
    class IVirtualFileSystem {
    public:
        virtual ~IVirtualFileSystem() = default;
        // Prevent copying/moving (Rule of Five)
        // IVirtualFileSystem(const IVirtualFileSystem&)            = delete;
        // IVirtualFileSystem& operator=(const IVirtualFileSystem&) = delete;
        // IVirtualFileSystem(IVirtualFileSystem&&)                 = delete;
        // IVirtualFileSystem& operator=(IVirtualFileSystem&&)      = delete;

        /// @brief Try to open a file in the VFS at the specified path and file access mode
        /// @param _path the path to open the file from
        /// @param _mode the file access mode
        /// @return requested file present at the path
        virtual UniquePtr<IFile> OpenFile(const String& _path, EFileMode _mode) = 0;
        virtual bool Exists(const String& _path) = 0;
    };
} // namespace Umbra::Platform
