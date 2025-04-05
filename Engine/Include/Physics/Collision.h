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
        bool CheckCircleCircleOverlap(Math::Vector2f _positionA, Math::Vector2f _positionB, float _radiusA,
            float _radiusB, Collision& _collision);
    };

    class ContactResolver {
    public:
        void ResolveContacts(Vector<Collision>& _collisions, double _deltaTime);
        class BaseView* mBaseView;

    private:
        void ResolveContact(ContactPoint* _contact, EntityID _entityA, EntityID _entityB, double _deltaTime);
        void CalculateSeparatingVelocity(ContactPoint* _contact, PhysicsBodyComponent* _physicsBodyA,
            PhysicsBodyComponent* _physicsBodyB, double _deltaTime);
        void ResolvePenetration(ContactPoint* _contact, PhysicsBodyComponent* _physicsBodyA,
            TransformComponent* _transformA, PhysicsBodyComponent* _physicsBodyB, TransformComponent* _transformB);

    private:
        int mMaxIteration = 1;
    };
} // namespace Umbra
