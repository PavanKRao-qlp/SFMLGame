#pragma once
#include "ECS/Component.h"
#include "ECS/Components/LifeTime.h"
#include "ECS/Enity.h"
#include "ECS/System.h"
#include "Umbra.h"

namespace Umbra {
    class LifeTimeSystem : public System {
    public:
        inline LifeTimeSystem() : System(std::make_unique<ECView<LifeTimeComponent>>()) {}
        inline ~LifeTimeSystem() {}
        inline void Update() override {
            for (EntityID entity : mView->mEntities) {
                LifeTimeComponent* lifetime = mView->ecsRegister->GetComponent<LifeTimeComponent>(entity);
                lifetime->TimeSpent += GEngineStatics.GameConfig->FixedDeltaTime;
                if (lifetime->TimeSpent >= lifetime->Time) {
                    mView->ecsRegister->DestroyEntity(entity);
                }
            }
        }
    };
} // namespace Umbra
