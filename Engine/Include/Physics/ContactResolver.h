#pragma once
#include "ECS/Components/Collider.h"
#include "ECS/Components/PhysicsBodyComponent.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Enity.h"
#include "Math/Polygon.h"
#include "Physics/Collision.h"
#include "Umbra.h"

namespace Umbra {
    class ContactResolver {
    public:
        void ResolveContacts(Vector<Collision>& _collisions, double _deltaTime);
        class BaseView* mBaseView;

    private:
        void ResolveContact(ContactPoint* _contact, EntityID _entityA, EntityID _entityB, double _deltaTime);
        /* Calculate the relative velocity of the particles after collision
        based on Total momentum before collision = Total momentum after collision
          m₁vi + m₂ui = m₁vf + m₂uf
        */
        void CalculateSeparatingVelocity(ContactPoint* _contact, PhysicsBodyComponent* _physicsBodyA,
            PhysicsBodyComponent* _physicsBodyB, double _deltaTime);
        void ResolvePenetration(ContactPoint* _contact, PhysicsBodyComponent* _physicsBodyA,
            TransformComponent* _transformA, PhysicsBodyComponent* _physicsBodyB, TransformComponent* _transformB);

    private:
        int mMaxIteration = 1;
    };
} // namespace Umbra
