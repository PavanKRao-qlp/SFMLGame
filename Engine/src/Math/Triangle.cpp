#include "Math/Triangle.h"


namespace Umbra::Math {
    Triangle::Triangle(Vector2f _a, Vector2f _b, Vector2f _c) : Polygon() {
        mVertices.emplace_back(_a);
        mVertices.emplace_back(_b);
        mVertices.emplace_back(_c);
    }
    Triangle::Triangle() : Polygon() {}
    Triangle::~Triangle() {}
} // namespace Umbra::Math
