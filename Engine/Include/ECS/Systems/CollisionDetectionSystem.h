#pragma once
#include "Core/Clock.h"
#include "ECS/Component.h"
#include "ECS/Components/CollisionBox.h"
#include "ECS/Components/CollisionEvent.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Enity.h"
#include "ECS/System.h"
#include "Math/CollisionSystem.h"

namespace Umbra {
    class CollisionDetectionSystem : public System {
    public:
        inline CollisionDetectionSystem() : System(new ECView<CollisionBoxComponent, TransformComponent>()) {}
        inline ~CollisionDetectionSystem() {};
        inline void Update() override {
            for (EntityID entity : mView->mEntities) {
                for (EntityID entityOther : mView->mEntities) {
                    if (entity == entityOther) {
                        continue;
                    }
                    CollisionBoxComponent* collisionBox =
                        mView->ecsRegister->GetComponent<CollisionBoxComponent>(entity);
                    CollisionBoxComponent* collisionBoxOther =
                        mView->ecsRegister->GetComponent<CollisionBoxComponent>(entityOther);
                    if (Collision::CollisionSystem::CheckCollisionMask(
                            collisionBox->Channel, collisionBoxOther->Channel)
                        == false) {
                        continue;
                    }
                    TransformComponent* transform = mView->ecsRegister->GetComponent<TransformComponent>(entity);

                    TransformComponent* transformOther =
                        mView->ecsRegister->GetComponent<TransformComponent>(entityOther);

                    Math::Bounds2D transformedBounds(
                        transform->Position + collisionBox->Bounds2D.Center, collisionBox->Bounds2D.Size);
                    Math::Bounds2D transformedBoundsOther(transformOther->Position + collisionBoxOther->Bounds2D.Center,
                        collisionBoxOther->Bounds2D.Size);


                    if (transformedBounds.Intersects(transformedBoundsOther)) {

                        Collision::CollisionResponse Response;
                        Response.mEntityA              = entity;
                        Response.mEntityB              = entityOther;
                        Umbra::EntityID collisionEvent = mView->ecsRegister->CreateEntity();
                        mView->ecsRegister->AddComponent<CollisionEventComponent>(collisionEvent, Response);
                        mView->ecsRegister->AddTag(collisionEvent, "Collision");
                        // DrawDebugCollision(transformedBounds, transformedBoundsOther, entity, entityOther);
                    } else {
                        sf::VertexArray* lines = new sf::VertexArray(sf::LinesStrip, 2);
                        (*lines)[0].position =
                            sf::Vector2f((float) transformedBounds.Center.x, (float) transformedBounds.Center.y);
                        (*lines)[0].color    = sf::Color::Red;
                        (*lines)[1].position = sf::Vector2f(
                            (float) transformedBoundsOther.Center.x, (float) transformedBoundsOther.Center.y);
                        (*lines)[1].color = sf::Color::Red;

                        RenderSystem::DebugDrawCache.push_back(lines);
                    }
                }
            }
        }

        void DrawDebugCollision(Umbra::Math::Bounds2D& transformedBounds, Umbra::Math::Bounds2D& transformedBoundsOther,
            Umbra::EntityID entity, Umbra::EntityID entityOther) {
            sf::RectangleShape* quadShape = new sf::RectangleShape();
            quadShape->setSize(sf::Vector2f(transformedBounds.Size.x, transformedBounds.Size.y));
            quadShape->setPosition(
                sf::Vector2f((float) transformedBounds.Center.x, (float) transformedBounds.Center.y));
            quadShape->setOutlineThickness(1);
            quadShape->setFillColor(sf::Color::Transparent);
            quadShape->setOutlineColor(sf::Color::Green);
            quadShape->setOrigin(sf::Vector2f(transformedBounds.Size.x / 2, transformedBounds.Size.y / 2));

            sf::VertexArray* lines = new sf::VertexArray(sf::LinesStrip, 2);
            (*lines)[0].position = sf::Vector2f((float) transformedBounds.Center.x, (float) transformedBounds.Center.y);
            (*lines)[0].color    = sf::Color::Green;
            (*lines)[1].position =
                sf::Vector2f((float) transformedBoundsOther.Center.x, (float) transformedBoundsOther.Center.y);
            (*lines)[1].color = sf::Color::Green;


            RenderSystem::DebugDrawCache.push_back(quadShape);
            RenderSystem::DebugDrawCache.push_back(lines);
            TagComponent* t1 = mView->ecsRegister->HasComponent<TagComponent>(entity)
                                 ? mView->ecsRegister->GetComponent<TagComponent>(entity)
                                 : nullptr;
            TagComponent* t2 = mView->ecsRegister->HasComponent<TagComponent>(entityOther)
                                 ? mView->ecsRegister->GetComponent<TagComponent>(entityOther)
                                 : nullptr;
            String t1Tag     = (t1 != nullptr) ? t1->Tag : "t1 tag component not found!";
            String t2Tag     = (t2 != nullptr) ? t2->Tag : "t2 tag component not found!";
            Logger::Log(
                LogType::Trace, "Collision : > %i  %s  %i %s ", entity, t1Tag.c_str(), entityOther, t2Tag.c_str());
        }
    };


} // namespace Umbra
