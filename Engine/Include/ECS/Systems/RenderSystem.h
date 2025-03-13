#pragma once
#include "ECS/Component.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Enity.h"
#include "ECS/System.h"

namespace Umbra {
    class RenderSystem : public System {
    public:
        inline RenderSystem(sf::RenderWindow* _WindowHandle)
            : System(new ECView<TransformComponent, SpriteComponent>()) {
            mWindowHandle = _WindowHandle;
        }
        inline ~RenderSystem() {}
        inline void Update() override {
            mWindowHandle->clear(sf::Color::Black);
            for (EntityID entity : mView->mEntities) {
                const SpriteComponent* sprite       = mView->ecsRegister->GetComponent<SpriteComponent>(entity);
                const TransformComponent* transform = mView->ecsRegister->GetComponent<TransformComponent>(entity);
                sf::RectangleShape quadShape;
                // sf::Vector2<String>
                quadShape.setSize(sf::Vector2f(transform->Size.x, transform->Size.y));
                quadShape.setOrigin(transform->Pivot.x * transform->Size.x, transform->Pivot.y * quadShape.getSize().y);
                quadShape.setPosition(sf::Vector2f((float) transform->Position.x, (float) -transform->Position.y));
                quadShape.setRotation(transform->Angle);
                quadShape.setFillColor(sprite->color);
                if (sprite->refTexture != nullptr) {
                    quadShape.setTexture(sprite->refTexture->GetSfmlTexture());
                }
                mWindowHandle->draw(quadShape);
            };
            for (sf::Drawable* drawObj : RenderSystem::DebugDrawCache) {
                mWindowHandle->draw(*drawObj);
            }
            // RenderSystem::DebugDrawCache.clear();

            mWindowHandle->display();
        }

        static inline void DebugDrawLine(
            const Math::Vector2f& from, const Math::Vector2f& to, const sf::Color& color = sf::Color::Green) {
            // Create a vertex array to represent the line
            sf::VertexArray* lines = new sf::VertexArray(sf::LinesStrip, 2);
            // Set the starting position of the line
            (*lines)[0].position =
                sf::Vector2f((float) from.x, (float) -from.y); // Negate y for SFML's coordinate system
            (*lines)[0].color = color;
            // Set the end position of the line
            (*lines)[1].position = sf::Vector2f((float) to.x, (float) -to.y); // Negate y for SFML's coordinate system
            (*lines)[1].color    = color;

            // Add the line to the debug draw cache
            RenderSystem::DebugDrawCache.push_back(lines);
        }

        inline static Vector<sf::Drawable*> DebugDrawCache;

    protected:
        sf::RenderWindow* mWindowHandle = nullptr;
    };
} // namespace Umbra
