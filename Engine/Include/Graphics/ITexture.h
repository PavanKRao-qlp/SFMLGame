#pragma once
#include "EnginePCH.h"
#include "Math/Vector.h"

namespace Umbra {

    class ITexture {
    public:
        virtual ~ITexture()                           = default;
        virtual bool LoadFromFile(const String& _path) = 0;
        virtual void Unload()                          = 0;
        virtual void* GetNativeHandle() const          = 0;
        virtual Math::Vector2i GetSize() const         = 0;
    };

} // namespace Umbra
