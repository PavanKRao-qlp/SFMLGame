#pragma once
#include "EnginePCH.h"
#include "Math/Polygon.h"
#include "Math/Vector.h"


namespace Umbra::Math {
    class Triangle : public Polygon {
    public:
        Triangle();
        Triangle(Vector2f _a, Vector2f _b, Vector2f _c);
        ~Triangle();
    };
} // namespace Umbra::Math
