#pragma once
#include "ECS/Components/Collider.h"
#include "ECS/Components/PhysicsBodyComponent.h"
#include "ECS/Components/Transfrom.h"
#include "ECS/Enity.h"
#include "Umbra.h"

namespace Umbra {

    class ContactPoint {
    public:
        Math::Vector2f mContactNormal;
        Math::Vector2f mContactPosition;
        double mPenetration;
    };

    class Collision {
    public:
        EntityID mBodyA;
        EntityID mBodyB;
        double mCofOfRestitution;
        void AddContact(ContactPoint& _contact);
        Vector<ContactPoint>& GetContacts();

    private:
        Vector<ContactPoint> mContacts;
    };


    class CollisionDetector {
    public:
        Vector<Tuple<EntityID, EntityID>> RunBroadPhase();
        Vector<Collision> RunNarrowPhase(Vector<Tuple<EntityID, EntityID>>& PossibleCollisions);
        class BaseView* mBaseView;


    private:
        bool CheckCollision(EntityID _entityA, EntityID _entityB, Collision& _collision);
        bool CheckCircleCircleOverlap(Math::Vector2f _positionA, Math::Vector2f _positionB, float _radiusA,
            float _radiusB, Collision& _collision);
        bool CheckBoxBoxOverlapAABB(Math::Bounds2D _boundsA, Math::Bounds2D _boundsB, Collision& _collision);
    };

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
