#pragma once
#include "ECS/System.h"
#include "ECS/Component.h"
#include "ECS/Enity.h"
#include "ECS/Components/Transfrom.h"

namespace D2D
{
    class RotationSystem : public System
    {
    public:
        inline RotationSystem() : System(new ECView<TransformComponent>()) {
                                  };
        inline ~RotationSystem() {};
        inline void Update() override
        {
            for (EntityID entity : mView->mEntities)
            {
                TransformComponent *transform = mView->ecsRegister->GetComponent<TransformComponent>(entity);
                transform->angle += 0.01f;
                transform->y += 0.01f;
            };
        };

    protected:
    };
}