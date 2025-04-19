#include "Math/Box.h"

namespace Umbra::Math {

    Box::Box(const Vector2f& _position, const Vector2f& _size, float _angle)
        : Polygon(), mPosition(_position), mSize(_size), mAngle(_angle) {
        mVertices.emplace_back(_position + Vector2f(mSize.x * 0.5f, mSize.y * 0.5f).GetRotated(mAngle));
        mVertices.emplace_back(_position + Vector2f(mSize.x * 0.5f, -mSize.y * 0.5f).GetRotated(mAngle));
        mVertices.emplace_back(_position + Vector2f(-mSize.x * 0.5f, -mSize.y * 0.5f).GetRotated(mAngle));
        mVertices.emplace_back(_position + Vector2f(-mSize.x * 0.5f, mSize.y * 0.5f).GetRotated(mAngle));
        for (Vector2f vertex : mVertices) {
            mCenter += vertex;
        }
        mCenter /= mVertices.size() * 1.f;
    }

    // Vector<Vector2f> Box::GetNormals() {
    //     Vector<Vector2f> normals;
    //     normals.emplace_back(Vector2f(1, 0).GetRotated(mAngle));
    //     normals.emplace_back(Vector2f(0, 1).GetRotated(mAngle));
    //     return normals;
    // }
} // namespace Umbra::Math
