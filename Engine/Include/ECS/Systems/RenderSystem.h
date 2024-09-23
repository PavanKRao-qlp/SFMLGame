#pragma once
#include "ECS/System.h"
#include "ECS/Component.h"
#include "ECS/Enity.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/Transfrom.h"

namespace UMBRA
{
    class RenderSystem : public System
    {
    public:
        inline RenderSystem(sf::RenderWindow *_WindowHandle) : System(new ECView<TransformComponent, SpriteComponent>())
        {
            mWindowHandle = _WindowHandle;
        };
        inline ~RenderSystem() {};
        inline void Update() override
        {
            mWindowHandle->clear(sf::Color::Black);
            for (EntityID entity : mView->mEntities)
            {
                const SpriteComponent *sprite = mView->ecsRegister->GetComponent<SpriteComponent>(entity);
                const TransformComponent *transform = mView->ecsRegister->GetComponent<TransformComponent>(entity);
                sf::RectangleShape quadShape;
                //sf::Vector2<String>
                quadShape.setSize(sf::Vector2f(transform->Size.x, transform->Size.y));
                quadShape.setOrigin(transform->Pivot.x * transform->Size.x, transform->Pivot.y * quadShape.getSize().y);
                quadShape.setPosition(sf::Vector2f((float)transform->Position.x, (float)transform->Position.y));
                quadShape.setRotation(transform->Angle);
                quadShape.setFillColor(sprite->color);
                mWindowHandle->draw(quadShape);
            };
            mWindowHandle->display();
        };

    protected:
        sf::RenderWindow *mWindowHandle = nullptr;
    };
}