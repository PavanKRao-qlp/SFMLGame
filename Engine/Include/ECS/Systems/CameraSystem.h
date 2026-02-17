#pragma once
#include "Core/AppWindow.h"
#include "ECS/Component.h"
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/Transform.h"
#include "ECS/Enity.h"
#include "ECS/System.h"
#include "Graphics/IRenderDevice.h"
#include "Service/ServiceLocator.h"
#include "Umbra.h"

namespace Umbra {
    class CameraSystem : public System {
    public:
        inline CameraSystem(IRenderDevice* _renderDevice)
            : System(std::make_unique<ECView<CameraComponent, TransformComponent>>()) {
            mRenderDevice = _renderDevice;
        }
        inline ~CameraSystem() {}
        inline void SetScreenSize(Math::Vector2f _screenSize) {
            mScreenAspectRatio = _screenSize.x / _screenSize.y;
        }
        inline void SetRenderSize(Math::Vector2f _renderSize) {
            mRenderAspectRatio = _renderSize.x / _renderSize.y;
        }
        inline void Update() override {
            for (EntityID entity : mView->mEntities) {
                CameraComponent* camera       = mView->ecsRegister->GetComponent<CameraComponent>(entity);
                TransformComponent* transform = mView->ecsRegister->GetComponent<TransformComponent>(entity);
                if (camera->IsActive()) {
                    Math::Vector2f ViewPortSize = Math::Vector2f(
                        2 * camera->GetOrthographicSize() * mRenderAspectRatio, 2 * camera->GetOrthographicSize());
                    mRenderDevice->SetViewSize(ViewPortSize.x, ViewPortSize.y);

                    FloatRect ViewPortRect(0, 0, 1, 1);
                    if (mScreenAspectRatio > mRenderAspectRatio) {
                        ViewPortRect.Width = mRenderAspectRatio / mScreenAspectRatio;
                        ViewPortRect.Left  = (1 - ViewPortRect.Width) / 2.f;
                    } else {
                        ViewPortRect.Height = mScreenAspectRatio / mRenderAspectRatio;
                        ViewPortRect.Top    = (1 - ViewPortRect.Height) / 2.f;
                    }
                    mRenderDevice->SetViewport(ViewPortRect);
                    mRenderDevice->SetViewCenter(transform->Position.x, transform->Position.y);
                    mRenderDevice->ApplyView();

                    // Update RenderService with camera state for frustum culling
                    if (auto* rs = ServiceLocator::GetRenderService()) {
                        rs->SetCamera(transform->Position, camera->GetOrthographicSize(), mRenderAspectRatio);
                    }

                    break; // Only use the first active camera
                }
            }
        }

    protected:
        IRenderDevice* mRenderDevice = nullptr;
        float mRenderAspectRatio = 1.0f;
        float mScreenAspectRatio = 1.0f;
    };
} // namespace Umbra
