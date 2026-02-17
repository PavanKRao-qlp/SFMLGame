#pragma once
#include "ECS/Component.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/Transform.h"
#include "ECS/Enity.h"
#include "ECS/System.h"
#include "Service/Render/RenderService.h"
#include "Service/ServiceLocator.h"

namespace Umbra {

    class RenderSyncSystem : public System {
    public:
        inline RenderSyncSystem()
            : System(std::make_unique<ECView<TransformComponent, SpriteComponent>>()) {}

        inline ~RenderSyncSystem() {}

        inline void Update() override {
            RenderService* renderService = ServiceLocator::GetRenderService();
            if (!renderService) {
                return;
            }

            for (EntityID entity : mView->mEntities) {
                const SpriteComponent* sprite =
                    mView->ecsRegister->GetComponent<SpriteComponent>(entity);
                const TransformComponent* transform =
                    mView->ecsRegister->GetComponent<TransformComponent>(entity);

                RenderQuad quad;
                quad.Position   = transform->Position;
                quad.Size       = transform->Size;
                quad.Pivot      = transform->Pivot;
                quad.Angle      = transform->Angle;
                quad.ZOrder     = sprite->zOrder;
                quad.Tint       = sprite->color;
                quad.RefTexture = sprite->refTexture;
                quad.UVRect     = sprite->uvRect;

                renderService->SubmitQuad(quad);
            }
        }
    };

} // namespace Umbra
