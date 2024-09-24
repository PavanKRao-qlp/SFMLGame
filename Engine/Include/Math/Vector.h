#pragma once
#include "EnginePCH.h"

namespace Umbra::MATH {
    struct Vector2f {
    public:
        inline Vector2f() {};
        inline Vector2f(float _x, float _y) : x(_x), y(_y) {};
        float x;
        float y;
    };

    struct Vector2i {
    public:
        inline Vector2i() {};
        inline Vector2i(int32 _x, int32 _y) : x(_x), y(_y) {};
        int32 x = 0;
        int32 y = 0;
        operator Vector2f() {
            return Vector2f((float) x, (float) y);
        }
    };
} // namespace Umbra::MATH
