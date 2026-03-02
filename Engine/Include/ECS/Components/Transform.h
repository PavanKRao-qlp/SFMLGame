#pragma once
#include "ECS/Component.h"
#include "Math/Matrix3x3.h"
#include "Math/Vector.h"

namespace Umbra {

    // Unified transform component that carries local TRS, the world-space result
    // computed by SceneGraphSystem, cached matrices, and the parent/children links
    // needed to form a scene hierarchy.
    //
    // Semantic contract
    // -----------------
    //  Position / Angle / Scale  — local transform (relative to Parent).
    //                              For root entities (Parent == MAX_ENTITY) these
    //                              are identical to the world transform.
    //  WorldPosition / WorldAngle / WorldScale / WorldMatrix
    //                            — written every PreRender frame by SceneGraphSystem.
    //                              Treat as read-only from game / system code.
    //  Size / Pivot              — intrinsic shape data; not part of the TRS chain.
    //                              Render size = Size * WorldScale (applied in RenderSyncSystem).
    //  Parent / Children         — scene graph topology; use World::SetParent /
    //                              World::DetachFromParent to modify safely.
    struct TransformComponent : Component {
    public:
        // Backward-compatible constructor (maps to local transform).
        inline TransformComponent(Math::Vector2f _position, Math::Vector2f _size, float _angle = 0.f,
                                  Math::Vector2f _pivot = Math::Vector2f(0.5f, 0.5f))
            : Position(_position), Size(_size), Pivot(_pivot), Angle(_angle) {}

        // ---- Local transform (user-set, relative to parent) ----
        Math::Vector2f Position = {0.f, 0.f};
        float          Angle    = 0.f;          // degrees
        Math::Vector2f Scale    = {1.f, 1.f};   // local scale multiplier

        // ---- Shape (not propagated through hierarchy) ----
        Math::Vector2f Size  = {1.f, 1.f};
        Math::Vector2f Pivot = {0.5f, 0.5f};

        // ---- World transform (written by SceneGraphSystem every PreRender) ----
        Math::Vector2f   WorldPosition = {0.f, 0.f};
        float            WorldAngle    = 0.f;
        Math::Vector2f   WorldScale    = {1.f, 1.f};
        Math::Matrix3x3f LocalMatrix   = Math::Matrix3x3f::Identity();
        Math::Matrix3x3f WorldMatrix   = Math::Matrix3x3f::Identity();

        // ---- Scene hierarchy ----
        EntityID         Parent   = MAX_ENTITY;
        Vector<EntityID> Children;

        // ---- Helpers (use world-space angle for correct world-space directions) ----
        inline Math::Vector2f GetForward() const {
            return Math::Vector2f(Math::Cos(Math::DegreeToRadian(WorldAngle)),
                                  Math::Sin(Math::DegreeToRadian(WorldAngle)));
        }

        inline Math::Vector2f GetUp() const {
            return Math::Vector2f(Math::Cos(Math::DegreeToRadian(WorldAngle - 90.f)),
                                  Math::Sin(Math::DegreeToRadian(WorldAngle - 90.f)));
        }
    };

} // namespace Umbra
