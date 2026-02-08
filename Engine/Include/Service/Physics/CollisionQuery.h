#pragma once
#include "Math/Polygon.h"
#include "Math/Vector.h"
#include "Service/Physics/Collision.h"
#include "Service/Physics/PhysicsBody.h"

namespace Umbra::CollisionQuery {

    /// @brief Tests overlap between two shapes at given positions
    bool CheckCollision(const PhysicsBodyData& _a, const Umbra::PhysicsBodyData& _b, CollisionDef& _collisionDef);

    /// @brief Circle vs Circle overlap test
    bool TestCircleCircle(
        Math::Vector2f _posA, float _radiusA, Math::Vector2f _posB, float _radiusB, CollisionDef& _collisionDef);

    /// @brief Circle vs Circle overlap test
    bool TestCircleVsOBB(Math::Vector2f _posA, float _radiusA, Math::Vector2f _posB, Math::Vector2f _sizeB,
        float _angleB, CollisionDef& _collisionDef);

    /// @brief Circle vs Circle overlap test
    bool TestBoxVsBoxSAT(Math::Vector2f _posA, Math::Vector2f _sizeA, float _angleA, Math::Vector2f _posB,
        Math::Vector2f _sizeB, float _angleB, CollisionDef& _collisionDef);

    /// @brief Checks if 2 polygon are overlapping via SAT algorithm
    /// @param _shapeA polygon data for body A
    /// @param _shapeB polygon data for body B
    /// @param _collisionDef result Collision def
    /// @return result of overlap test , true if overlapping
    bool TestPolygonVsPolygonOverlapSAT(Math::Polygon& _shapeA, Math::Polygon& _shapeB, CollisionDef& _collisionDef);

    /// @brief Gets Contact Points for SAT
    /// @param _shapeA polygon data for body A
    /// @param _shapeB polygon data for body B
    /// @param _normal MTV of collision
    /// @return returns contact points
    Vector<ContactDef> GetContactPointsViaClipping(
        Math::Polygon& _shapeA, Math::Polygon& _shapeB, Math::Vector2f& _normal);

    /// @brief
    /// @param _normal
    /// @param _edge
    /// @param _clippingPlaneProj
    /// @return
    int Clip(const Math::Vector2f& _normal, Pair<Math::Vector2f, Math::Vector2f>& _edge, float _clippingPlaneProj);


} // namespace Umbra::CollisionQuery
