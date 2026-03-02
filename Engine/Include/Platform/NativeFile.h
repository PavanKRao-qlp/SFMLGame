#pragma once
#include "Umbra.h"
#include "Platform/VirtualFileSystem.h"
#include <fstream>
#include <sys/stat.h>

namespace Umbra::Platform {
    class NativeFile : public IFile {
    public:
        NativeFile(const String& _path, EFileMode _mode);
        virtual uint64 Read(void* _buffer, uint64 _size) override;
        virtual uint64 Write(const void* _buffer, uint64 _size) override;
        virtual bool Seek(uint64 _offset) override;
        virtual uint64 Tell() override;
        virtual uint64 Size() override;
        virtual bool Close() override;
        virtual bool IsValid() const override { return mFile.is_open(); }

    private:
        std::fstream mFile;
        String mPath;
    };
} // namespace Umbra::Platform
