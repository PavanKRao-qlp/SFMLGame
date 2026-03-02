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

    // ============== CCD Time-of-Impact Functions ==============

    /// @brief Computes earliest time of impact between two moving circles
    /// @return TOI in [0,1], where 1.0 means no collision during the sweep
    float TimeOfImpactCircleCircle(Math::Vector2f _posA0, Math::Vector2f _posA1, float _radiusA,
        Math::Vector2f _posB, float _radiusB);

    /// @brief Computes earliest time of impact for a moving circle against a static OBB
    /// @return TOI in [0,1], where 1.0 means no collision during the sweep
    float TimeOfImpactCircleOBB(Math::Vector2f _posA0, Math::Vector2f _posA1, float _radiusA,
        Math::Vector2f _posB, Math::Vector2f _sizeB, float _angleB);

    /// @brief Computes earliest time of impact for a moving box against a static box via bisection
    /// @return TOI in [0,1], where 1.0 means no collision during the sweep
    float TimeOfImpactBoxBox(Math::Vector2f _posA0, Math::Vector2f _posA1, Math::Vector2f _sizeA, float _angleA,
        Math::Vector2f _posB, Math::Vector2f _sizeB, float _angleB, int _bisectionIterations);

    /// @brief Dispatcher that selects the correct TOI function based on body shapes
    /// @param _bodyA The CCD body (swept from _oldPosA to its current position)
    /// @param _oldPosA Pre-integration position of bodyA
    /// @param _bodyB The candidate body at its current position
    /// @param _bisectionIterations Number of bisection iterations for box-box
    /// @return TOI in [0,1], where 1.0 means no collision during the sweep
    float ComputeTimeOfImpact(const PhysicsBodyData& _bodyA, Math::Vector2f _oldPosA,
        const PhysicsBodyData& _bodyB, int _bisectionIterations);

} // namespace Umbra::CollisionQuery
