#pragma once
#include "ECS/Component.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Enity.h"
#include "ECS/System.h"

namespace Umbra {
    class RotationSystem : public System {
    public:
        inline RotationSystem() : System(new ECView<TransformComponent>()) {
            bEnabled = false;
        }
        inline ~RotationSystem() {};
        inline void Update() override {
            for (EntityID entity : mView->mEntities) {
                TransformComponent* transform = mView->ecsRegister->GetComponent<TransformComponent>(entity);
                transform->Angle += 50.f * EngineTime::GetDeltaTime();
                // transform->y += 5 * EngineTime::GetDeltaTime();
            };
        }

    protected:
    };
} // namespace Umbra
