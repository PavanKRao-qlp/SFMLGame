#pragma once
#include "ECS/Components/CollisionCallback.h"
#include "ECS/ECSRegister.h"
#include "ECS/System.h"
#include "Service/Physics/PhysicsService.h"

namespace Umbra {

    class CollisionEventDispatchSystem : public System {
    public:
        explicit CollisionEventDispatchSystem(PhysicsService* _physicsService)
            : System(std::make_unique<ECView<CollisionCallbackComponent>>())
            , mPhysicsService(_physicsService) {}

        void Update() override {
            if (!mPhysicsService) {
                return;
            }

            auto* reg = mView->ecsRegister;

            // Dispatch Enter events
            for (const CollisionEvent& event : mPhysicsService->GetCollisionEnterEvents()) {
                EntityID entityA = ResolveEntity(event.HandleA);
                EntityID entityB = ResolveEntity(event.HandleB);

                if (entityA != UINT64_MAX && reg->HasComponent<CollisionCallbackComponent>(entityA)) {
                    auto* cb = reg->GetComponent<CollisionCallbackComponent>(entityA);
                    if (cb && cb->OnCollisionEnter) {
                        cb->OnCollisionEnter(entityA, entityB, event);
                    }
                }
                if (entityB != UINT64_MAX && reg->HasComponent<CollisionCallbackComponent>(entityB)) {
                    auto* cb = reg->GetComponent<CollisionCallbackComponent>(entityB);
                    if (cb && cb->OnCollisionEnter) {
                        cb->OnCollisionEnter(entityB, entityA, event);
                    }
                }
            }

            // Dispatch Stay events
            for (const CollisionEvent& event : mPhysicsService->GetCollisionStayEvents()) {
                EntityID entityA = ResolveEntity(event.HandleA);
                EntityID entityB = ResolveEntity(event.HandleB);

                if (entityA != UINT64_MAX && reg->HasComponent<CollisionCallbackComponent>(entityA)) {
                    auto* cb = reg->GetComponent<CollisionCallbackComponent>(entityA);
                    if (cb && cb->OnCollisionStay) {
                        cb->OnCollisionStay(entityA, entityB, event);
                    }
                }
                if (entityB != UINT64_MAX && reg->HasComponent<CollisionCallbackComponent>(entityB)) {
                    auto* cb = reg->GetComponent<CollisionCallbackComponent>(entityB);
                    if (cb && cb->OnCollisionStay) {
                        cb->OnCollisionStay(entityB, entityA, event);
                    }
                }
            }

            // Dispatch Exit events
            for (const CollisionEvent& event : mPhysicsService->GetCollisionExitEvents()) {
                EntityID entityA = ResolveEntity(event.HandleA);
                EntityID entityB = ResolveEntity(event.HandleB);

                if (entityA != UINT64_MAX && reg->HasComponent<CollisionCallbackComponent>(entityA)) {
                    auto* cb = reg->GetComponent<CollisionCallbackComponent>(entityA);
                    if (cb && cb->OnCollisionExit) {
                        cb->OnCollisionExit(entityA, entityB);
                    }
                }
                if (entityB != UINT64_MAX && reg->HasComponent<CollisionCallbackComponent>(entityB)) {
                    auto* cb = reg->GetComponent<CollisionCallbackComponent>(entityB);
                    if (cb && cb->OnCollisionExit) {
                        cb->OnCollisionExit(entityB, entityA);
                    }
                }
            }
        }

    private:
        EntityID ResolveEntity(BodyHandle _handle) const {
            if (!mPhysicsService->IsBodyValid(_handle)) {
                return UINT64_MAX;
            }
            void* userData = mPhysicsService->GetUserData(_handle);
            if (!userData) {
                return UINT64_MAX;
            }
            return static_cast<EntityID>(reinterpret_cast<uintptr_t>(userData));
        }

        PhysicsService* mPhysicsService = nullptr;
    };

} // namespace Umbra
