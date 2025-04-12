#pragma once
#include "Core/Clock.h"
#include "ECS/Component.h"
#include "ECS/Components/SpriteQuad.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Enity.h"
#include "ECS/System.h"
#include "Math/Bounds.h"
#include "UI/ImGuiBackend.h"

namespace Umbra {
    class RenderSystem : public System {
    public:
        inline RenderSystem(sf::RenderWindow* _windowHandle, ImGuiBackend* _imGuiBackend)
            : System(new ECView<TransformComponent, SpriteComponent>()) {
            mWindowHandle = _windowHandle;
            mImguiBackend = _imGuiBackend;
        }
        inline ~RenderSystem() {}
        inline void Update() override {
            for (EntityID entity : mView->mEntities) {
                const SpriteComponent* sprite       = mView->ecsRegister->GetComponent<SpriteComponent>(entity);
                const TransformComponent* transform = mView->ecsRegister->GetComponent<TransformComponent>(entity);
                sf::RectangleShape quadShape;
                // sf::Vector2<String>
                quadShape.setSize(sf::Vector2f(transform->Size.x, transform->Size.y));
                quadShape.setOrigin(transform->Pivot.x * transform->Size.x, transform->Pivot.y * quadShape.getSize().y);
                quadShape.setPosition(sf::Vector2f((float) transform->Position.x, (float) -transform->Position.y));
                quadShape.setRotation(-transform->Angle);
                quadShape.setFillColor(sprite->color);
                if (sprite->refTexture != nullptr) {
                    quadShape.setTexture(sprite->refTexture->GetSfmlTexture());
                }
                mWindowHandle->draw(quadShape);
            }
            for (sf::Drawable* drawObj : RenderSystem::DebugDrawCache) {
                mWindowHandle->draw(*drawObj);
            }
            // RenderSystem::DebugDrawCache.clear();

            mWindowHandle->display();
            // mImguiBackend->Render();
        }

        static inline void DebugDrawLine(
            const Math::Vector2f& _from, const Math::Vector2f& _to, const sf::Color& _color = sf::Color::Green) {
            // Create a vertex array to represent the line
            sf::VertexArray* lines = new sf::VertexArray(sf::LinesStrip, 2);
            // Set the starting position of the line
            (*lines)[0].position =
                sf::Vector2f((float) _from.x, (float) -_from.y); // Negate y for SFML's coordinate system
            (*lines)[0].color = _color;
            // Set the end position of the line
            (*lines)[1].position = sf::Vector2f((float) _to.x, (float) -_to.y); // Negate y for SFML's coordinate system
            (*lines)[1].color    = _color;

            // Add the line to the debug draw cache
            RenderSystem::DebugDrawCache.push_back(lines);
        }

        static inline void DebugDrawCircle(const Math::Vector2f& _position, const float _radius, bool _bFilled = false,
            const sf::Color& _color = sf::Color::White) {
            sf::CircleShape* shape = new sf::CircleShape(_radius);
            shape->setPosition(sf::Vector2f((float) _position.x - _radius, (float) -_position.y - _radius));
            if (_bFilled) {
                shape->setFillColor(_color);
            } else {
                shape->setOutlineThickness(0.2f);
                shape->setOutlineColor(_color);
                shape->setFillColor(sf::Color::Transparent);
            }
            RenderSystem::DebugDrawCache.push_back(shape);
        }

        static inline void DrawDebugBox(
            const Math::Bounds2D& _bounds, bool _bFilled = false, const sf::Color& _color = sf::Color::White) {
            sf::RectangleShape* shape = new sf::RectangleShape();
            shape->setSize(sf::Vector2f(_bounds.Size.x, _bounds.Size.y));
            shape->setOrigin(sf::Vector2f(_bounds.Size.x * 0.5f, _bounds.Size.y * 0.5f));
            shape->setPosition(sf::Vector2f(_bounds.Center.x, -1 * _bounds.Center.y));
            if (_bFilled) {
                shape->setFillColor(_color);
            } else {
                shape->setOutlineThickness(0.5f);
                shape->setOutlineColor(_color);
                shape->setFillColor(sf::Color::Transparent);
            }
            RenderSystem::DebugDrawCache.push_back(shape);
        }

        static inline void DrawDebugOrientedBox(const Math::Bounds2D& _bounds, float _angle, bool _bFilled = false,
            const sf::Color& _color = sf::Color::White) {
            sf::RectangleShape* shape = new sf::RectangleShape();
            shape->setSize(sf::Vector2f(_bounds.Size.x, _bounds.Size.y));
            shape->setOrigin(sf::Vector2f(_bounds.Size.x * 0.5f, _bounds.Size.y * 0.5f));
            shape->setPosition(sf::Vector2f(_bounds.Center.x, -1 * _bounds.Center.y));
            shape->setRotation(-_angle);
            if (_bFilled) {
                shape->setFillColor(_color);
            } else {
                shape->setOutlineThickness(0.5f);
                shape->setOutlineColor(_color);
                shape->setFillColor(sf::Color::Transparent);
            }
            RenderSystem::DebugDrawCache.push_back(shape);
        }

        inline static Vector<sf::Drawable*> DebugDrawCache;

    protected:
        sf::RenderWindow* mWindowHandle = nullptr;
        ImGuiBackend* mImguiBackend     = nullptr;
    };
} // namespace Umbra
