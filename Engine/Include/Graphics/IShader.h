#pragma once
#include "EnginePCH.h"

namespace Umbra {

    class IShader {
    public:
        virtual ~IShader() = default;

        virtual bool LoadFragment(const String& _fragPath)                          = 0;
        virtual bool LoadVertFrag(const String& _vertPath, const String& _fragPath) = 0;
        virtual void Unload()                                                        = 0;
        virtual void* GetNativeHandle() const                                        = 0;

        virtual void SetUniformFloat(const String& _name, float _val)                                  = 0;
        virtual void SetUniformInt(const String& _name, int _val)                                      = 0;
        virtual void SetUniformBool(const String& _name, bool _val)                                    = 0;
        virtual void SetUniformVec2(const String& _name, float _x, float _y)                          = 0;
        virtual void SetUniformVec3(const String& _name, float _x, float _y, float _z)                = 0;
        virtual void SetUniformVec4(const String& _name, float _x, float _y, float _z, float _w)      = 0;
    };

} // namespace Umbra
