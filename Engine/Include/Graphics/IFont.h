#pragma once
#include "EnginePCH.h"

namespace Umbra {

    class IFont {
    public:
        virtual ~IFont()                            = default;
        virtual bool LoadFromFile(const String& _path) = 0;
        virtual void Unload()                          = 0;
        virtual void* GetNativeHandle() const          = 0;
    };

} // namespace Umbra
