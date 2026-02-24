#include "ECS/Systems/SceneGraphSystem.h"
#include "ECS/Components/Transform.h"
#include "ECS/View.h"

namespace Umbra {

    SceneGraphSystem::SceneGraphSystem()
        : System(std::make_unique<ECView<TransformComponent>>()) {}

    void SceneGraphSystem::Update() {
        // Pass 1: rebuild every entity's LocalMatrix from its local TRS fields.
        for (EntityID entity : mView->mEntities) {
            TransformComponent* t = mView->ecsRegister->GetComponent<TransformComponent>(entity);
            t->LocalMatrix        = Math::Matrix3x3f::TRS(t->Position, t->Angle, t->Scale);
        }

        // Pass 2: DFS from each root entity to propagate WorldMatrix down the tree.
        const Math::Matrix3x3f identity = Math::Matrix3x3f::Identity();
        for (EntityID entity : mView->mEntities) {
            TransformComponent* t = mView->ecsRegister->GetComponent<TransformComponent>(entity);
            if (t->Parent == MAX_ENTITY) {
                ProcessHierarchy(entity, identity);
            }
        }
    }

    void SceneGraphSystem::ProcessHierarchy(EntityID _entity, const Math::Matrix3x3f& _parentWorld) {
        TransformComponent* t = mView->ecsRegister->GetComponent<TransformComponent>(_entity);
        if (!t)
            return;

        t->WorldMatrix   = _parentWorld * t->LocalMatrix;
        t->WorldPosition = t->WorldMatrix.GetTranslation();
        t->WorldAngle    = t->WorldMatrix.GetRotationDeg();
        t->WorldScale    = t->WorldMatrix.GetScale();

        for (EntityID child : t->Children) {
            ProcessHierarchy(child, t->WorldMatrix);
        }
    }

} // namespace Umbra
