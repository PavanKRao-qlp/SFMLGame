#pragma once
#include <filesystem>
#include "EnginePCH.h"

namespace Umbra::PathUtils {

// Normalize path separators to forward slashes and resolve . / .. segments.
inline String Normalize(const String& _path)
{
    namespace fs = std::filesystem;
    String result = fs::path(_path).lexically_normal().generic_string();
    return result;
}

// Return the parent directory of a path (empty string if none).
// "assets/textures/hero.png" -> "assets/textures"
inline String GetDirectory(const String& _path)
{
    namespace fs = std::filesystem;
    return fs::path(_path).parent_path().generic_string();
}

// Return the filename including extension.
// "assets/textures/hero.png" -> "hero.png"
inline String GetFileName(const String& _path)
{
    namespace fs = std::filesystem;
    return fs::path(_path).filename().generic_string();
}

// Return the filename without extension (stem).
// "assets/textures/hero.png" -> "hero"
inline String GetStem(const String& _path)
{
    namespace fs = std::filesystem;
    return fs::path(_path).stem().generic_string();
}

// Return the extension including the leading dot.
// "assets/textures/hero.png" -> ".png"
inline String GetExtension(const String& _path)
{
    namespace fs = std::filesystem;
    return fs::path(_path).extension().generic_string();
}

// Join two path segments and normalize the result.
// Join("assets/textures", "hero.png") -> "assets/textures/hero.png"
inline String Join(const String& _base, const String& _relative)
{
    namespace fs = std::filesystem;
    return (fs::path(_base) / _relative).lexically_normal().generic_string();
}

// Replace (or add) the extension on a path.
// ChangeExtension("hero.png", ".jpg") -> "hero.jpg"
// ChangeExtension("hero",     ".png") -> "hero.png"
inline String ChangeExtension(const String& _path, const String& _newExt)
{
    namespace fs = std::filesystem;
    return fs::path(_path).replace_extension(_newExt).generic_string();
}

// Convert a relative path to an absolute path based on the current working directory.
inline String GetAbsolute(const String& _path)
{
    namespace fs = std::filesystem;
    return fs::absolute(_path).generic_string();
}

// Get the path of _to relative to the directory _from.
// Both paths should be absolute for a meaningful result.
// GetRelative("assets/textures", "assets/audio/sfx.wav") -> "../audio/sfx.wav"
inline String GetRelative(const String& _from, const String& _to)
{
    namespace fs = std::filesystem;
    return fs::path(_to).lexically_relative(fs::path(_from)).generic_string();
}

// True if _path is absolute (e.g. "C:/..." or "/usr/...").
inline bool IsAbsolute(const String& _path)
{
    return std::filesystem::path(_path).is_absolute();
}

// True if something exists at _path (file or directory).
inline bool Exists(const String& _path)
{
    return std::filesystem::exists(_path);
}

// True if _path refers to a regular file.
inline bool IsFile(const String& _path)
{
    return std::filesystem::is_regular_file(_path);
}

// True if _path refers to a directory.
inline bool IsDirectory(const String& _path)
{
    return std::filesystem::is_directory(_path);
}

} // namespace Umbra::PathUtils
