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
        /**
         * @brief Projection of Polygon vertices
         */
        struct Projection {
            float Min;
            float Max;
        };

        Polygon();
        Polygon(Vector<Math::Vector2f>& _vertices);
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
        Vector<Math::Vector2f> GetEdges();


        /**
         * @brief Returns the centroid of the polygon
         */
        Math::Vector2f GetCenter();

        /**
         * @brief Projects all vertices onto axis and return min to max projection.
         */
        Projection GetProjectionOntoAxis(Math::Vector2f _axis);

    protected:
        /// List of 2D points defining the polygon (assumed to be in order, forming edges)
        Vector<Math::Vector2f> mVertices;
        // centroid position vector
        Math::Vector2f mCenter;
    };

} // namespace Umbra::Math
