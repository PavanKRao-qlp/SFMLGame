#include "Platform/NativeFile.h"

namespace Umbra::Platform {

    NativeFile::NativeFile(const String& _path, EFileMode _mode) : IFile() {
        mPath                       = _path;
        std::ios::openmode openmode = std::ios::binary;
        if (_mode == EFileMode::Read) {
            openmode = std::ios::in;
        } else if (_mode == EFileMode::Write) {
            openmode = std::ios::out | std::ios::trunc;
        } else if (_mode == EFileMode::ReadBinary) {
            openmode = std::ios::in | std::ios::binary;
        } else if (_mode == EFileMode::WriteBinary) {
            openmode = std::ios::out | std::ios::binary | std::ios::trunc;
        }
        mFile.open(_path, openmode);
        if (!mFile.is_open()) {
            UMBRA_LOG_ERROR("File failed to open : %s", _path.c_str());
            return;
        }
    }

    uint64 NativeFile::Read(void* _buffer, uint64 _size) {
        if (!mFile.read(reinterpret_cast<char*>(_buffer), _size)) {
            if (!mFile.eof()) {
                UMBRA_LOG_ERROR("File failed to read : %s", mPath.c_str());
            }
        }
        return mFile.gcount();
    }

    uint64 NativeFile::Write(const void* _buffer, uint64 _size) {
        mFile.write(reinterpret_cast<const char*>(_buffer), _size);
        if (!mFile) {
            UMBRA_LOG_ERROR("File failed to write : %s", mPath.c_str());
            return 0;
        }
        return _size;
    }

    bool NativeFile::Seek(uint64 _offset) {
        mFile.seekg(_offset);
        mFile.seekp(_offset);
        if (!mFile) {
            UMBRA_LOG_ERROR("File failed to seek : %s", mPath.c_str());
            return false;
        }
        return true;
    }

    uint64 NativeFile::Tell() {
        return static_cast<uint64>(mFile.tellg());
    }

    uint64 NativeFile::Size() {
        uint64 currentPos = static_cast<uint64>(mFile.tellg());
        mFile.seekg(0, std::ios::end); // go to end of file
        auto endPos = static_cast<uint64>(mFile.tellg());
        mFile.seekg(currentPos); // go back to current cursor offset
        return endPos;
    }

    bool NativeFile::Close() {
        mFile.close();
        return !mFile.is_open();
    }

} // namespace Umbra::Platform
