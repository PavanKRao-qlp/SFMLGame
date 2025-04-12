#pragma once
#include "EnginePCH.h"
#include "Math/Vector.h"

namespace Umbra::Math {
    /**
     * @brief Represents a 2D polygon defined by a list of vertices.
     *
     * Provides utility functions to retrieve its vertices, edges, and surface normals.
     */
    class Polygon {
    public:
        Polygon(Vector<Math::Vector2f>& _vertices) : mVertices(_vertices) {}
        ~Polygon();

        /**
         * @brief Returns the list of surface normals (perpendicular vectors to each edge).
         *
         * Normals are calculated using clockwise or counter-clockwise winding.
         */
        virtual Vector<Math::Vector2f> GetNormals();

        /**
         * @brief Returns the list of vertices that define the polygon.
         */
        Vector<Math::Vector2f> GetVertices();

        /**
         * @brief Returns the list of edges as vectors (vertex[i+1] - vertex[i]).
         */
        virtual Vector<Math::Vector2f> GetEdges();

    private:
        /// List of 2D points defining the polygon (assumed to be in order, forming edges)
        Vector<Math::Vector2f> mVertices;
    };

} // namespace Umbra::Math
