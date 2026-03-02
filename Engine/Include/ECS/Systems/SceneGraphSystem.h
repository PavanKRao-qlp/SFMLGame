#pragma once
#include "ECS/Components/Transform.h"
#include "ECS/Enity.h"
// View.h → ECSRegister.h → System.h (defines ESystemPhase/System before ECSRegister needs them).
// Must come before System.h to avoid the System.h → ECSRegister.h circular-include issue.
#include "ECS/View.h"
#include "ECS/System.h"
#include "Math/Matrix3x3.h"

namespace Umbra {

    // Runs at PreRender priority -5 (before CameraSystem and RenderSyncSystem).
    //
    // Each frame it:
    //   1. Rebuilds every entity's LocalMatrix from (Position, Angle, Scale).
    //   2. DFS-walks the scene graph from each root entity (Parent == MAX_ENTITY),
    //      concatenates WorldMatrix = parentWorldMatrix * LocalMatrix, then
    //      decomposes to WorldPosition / WorldAngle / WorldScale.
    //
    // Root entities get WorldMatrix == LocalMatrix, so their WorldPosition equals
    // their Position — existing code that only uses root entities is unaffected.
    class SceneGraphSystem : public System {
    public:
        SceneGraphSystem();
        void Update() override;

    private:
        void ProcessHierarchy(EntityID _entity, const Math::Matrix3x3f& _parentWorld);
    };

} // namespace Umbra
