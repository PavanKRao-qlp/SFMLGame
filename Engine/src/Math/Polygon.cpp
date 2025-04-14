#include "Math/Polygon.h"

namespace Umbra::Math {

    Vector<Vector2f> Polygon::GetVertices() {
        return mVertices;
    }

    Vector<Vector2f> Polygon::GetEdges() {
        Vector<Vector2f> edges;
        for (int i = 0; i < mVertices.size(); i++) {
            Vector2f edge = mVertices[(i + 1) % mVertices.size()] - mVertices[i];
            edges.emplace_back(edge);
        }

        return edges;
    }

    Vector<Vector2f> Polygon::GetNormals() {
        Vector<Vector2f> normals;
        for (int i = 0; i < mVertices.size(); i++) {
            Vector2f edge = mVertices[(i + 1) % mVertices.size()] - mVertices[i];
            normals.emplace_back(Vector2f::Perpendicular(edge.GetNormalized()));
        }
        return normals;
    }
    Polygon::~Polygon() {}


    Polygon::Polygon() {}
} // namespace Umbra::Math
