#pragma once
#include "ECS/System.h"
#include "ECS/Component.h"
#include "ECS/Enity.h"
#include "ECS/Components/Transfrom.h"

namespace UMBRA
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
                transform->angle += 50.f * EngineTime::GetDeltaTime();
                //transform->y += 5 * EngineTime::GetDeltaTime();
            };
        };

    protected:
    };
}