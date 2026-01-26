#pragma once
#include "Core/AppWindow.h"
#include "ECS/Component.h"
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Enity.h"
#include "ECS/System.h"
#include "Umbra.h"

#include "SFML/Graphics.hpp"
namespace Umbra {
    class CameraSystem : public System {
    public:
        inline CameraSystem(sf::View* _renderView)
            : System(std::make_unique<ECView<CameraComponent, TransformComponent>>()) {
            mRenderView = _renderView;
        }
        inline ~CameraSystem() {}
        inline void SetScreenSize(Math::Vector2f _screenSize) {
            mScreenAspectRatio = _screenSize.x / _screenSize.y;
        }
        inline void SetRenderSize(Math::Vector2f _renderSize) {
            mRenderAspectRatio = _renderSize.x / _renderSize.y;
        }
        inline void Update() override {
            EntityID activeCam = MAX_ENTITY;
            for (EntityID entity : mView->mEntities) {
                CameraComponent* camera       = mView->ecsRegister->GetComponent<CameraComponent>(entity);
                TransformComponent* transform = mView->ecsRegister->GetComponent<TransformComponent>(entity);
                if (camera->IsActive()) {
                    Math::Vector2f ViewPortSize = Math::Vector2f(
                        2 * camera->GetOrthographicSize() * mRenderAspectRatio, 2 * camera->GetOrthographicSize());
                    mRenderView->setSize(ViewPortSize.x, ViewPortSize.y);
                    sf::FloatRect ViewPortRect(0, 0, 1, 1);
                    if (mScreenAspectRatio > mRenderAspectRatio) {
                        //  scale render width down by 1/AR to match it to height
                        ViewPortRect.width = mRenderAspectRatio / mScreenAspectRatio;
                        ViewPortRect.left  = (1 - ViewPortRect.width) / 2.f;
                    } else {
                        //  scale render height down by AR to match it to height
                        ViewPortRect.height = mScreenAspectRatio / mRenderAspectRatio;
                        ViewPortRect.top    = (1 - ViewPortRect.height) / 2.f;
                    }
                    mRenderView->setViewport(ViewPortRect);
                    mRenderView->setCenter(transform->Position.x, transform->Position.y);
                    GEngineStatics.AppWindowPtr->GetRenderWindowHandle()->setView(*mRenderView);
                }
            }
        }

    protected:
        sf::View* mRenderView = nullptr;
        float mRenderAspectRatio;
        float mScreenAspectRatio;
    };
} // namespace Umbra
