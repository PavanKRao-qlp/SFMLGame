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

    Polygon::Projection Polygon::GetProjectionOntoAxis(Math::Vector2f _axis) {
        Projection projection;
        projection.Min = fInf;
        projection.Max = -fInf;
        for (Math::Vector2f vertex : mVertices) {
            float vertexProjection = Math::Vector2f::Dot(vertex, _axis);
            projection.Min         = Math::Min(projection.Min, vertexProjection);
            projection.Max         = Math::Max(projection.Max, vertexProjection);
        }
        return projection;
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
