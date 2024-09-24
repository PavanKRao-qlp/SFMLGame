#pragma once
#include "Core/Clock.h"
#include "ECS/Component.h"
#include "ECS/Components/LifeTime.h"
#include "ECS/Enity.h"
#include "ECS/System.h"

namespace Umbra {
    class LifeTimeSystem : public System {
    public:
        inline LifeTimeSystem() : System(new ECView<LifeTimeComponent>()) {};
        inline ~LifeTimeSystem() {};
        inline void Update() override {
            for (EntityID entity : mView->mEntities) {
                LifeTimeComponent* lifetime = mView->ecsRegister->GetComponent<LifeTimeComponent>(entity);
                lifetime->Time -= EngineTime::GetDeltaTime();
                if (lifetime->Time <= 0) {
                    mView->ecsRegister->DestroyEntity(entity);
                }
            }
        }
    };
} // namespace Umbra
