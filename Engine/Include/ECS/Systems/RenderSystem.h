#pragma once
#include "ECS/System.h"
#include "ECS/Component.h"
#include "ECS/Enity.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/Transfrom.h"

namespace D2D
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
                ComponentID ix = ComponentIDHelper::GetID<TransformComponent>();
                const SpriteComponent *sprite = mView->ecsRegister->GetComponent<SpriteComponent>(entity);
                const TransformComponent *transform = mView->ecsRegister->GetComponent<TransformComponent>(entity);
                sf::RectangleShape quadShape;
                quadShape.setSize(sf::Vector2f(100, 100));
                quadShape.setOrigin(transform->pivotX * quadShape.getSize().x, transform->pivotY * quadShape.getSize().y);
                quadShape.setPosition(sf::Vector2f((float)transform->x, (float)transform->y));
                quadShape.setRotation(transform->angle);
                quadShape.setFillColor(sprite->color);
                mWindowHandle->draw(quadShape);
            };
            mWindowHandle->display();
        };

    protected:
        sf::RenderWindow *mWindowHandle = nullptr;
    };
}