#pragma once
#include "EnginePCH.h"
#include "Math/MathUtils.h"
#include "Math/Polygon.h"
#include "Math/Vector.h"


namespace Umbra::Math {
    /**
     * @brief Represents an OBB
     */
    class Box : public Polygon {
    public:
        Box() {}
        Box(const Vector2f& _position, const Vector2f& _size, float _angle);
        // Box(const Vector2f& _position, const Vector2f& _xExtent, const Vector2f& _yExtent);
        // overriding since we know 2 normals are parallel
        //  virtual Vector<Math::Vector2f> GetNormals() override;

    private:
        float mAngle;
        Vector2f mPosition;
        Vector2f mSize;
    };

} // namespace Umbra::Math
