#pragma once
#include "EnginePCH.h"
#include "Math/MathUtils.h"
#include "Math/Vector.h"


namespace Umbra::Math {

    class Ray2D {
    public:
        Ray2D() {}
        Ray2D(const Vector2f& _position, const Vector2f& _dir) : Position(_position), Direction(_dir) {}

        Vector2f Position;
        Vector2f Direction;
    };

} // namespace Umbra::Math
