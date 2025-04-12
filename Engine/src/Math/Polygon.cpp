#include "Math/Polygon.h"

namespace Umbra::Math {

    Vector<Math::Vector2f> Polygon::GetNormals() {
        return Vector<Math::Vector2f>();
    }
    Vector<Math::Vector2f> Polygon::GetVertices() {
        return mVertices;
    }
    Vector<Math::Vector2f> Polygon::GetEdges() {
        return Vector<Math::Vector2f>();
    }

    Polygon::~Polygon() {}


} // namespace Umbra::Math
