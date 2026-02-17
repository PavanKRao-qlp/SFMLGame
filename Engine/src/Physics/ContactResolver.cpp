#include "Physics/ContactResolver.h"

#include "ECS/ECSRegister.h"
#include "Service/ServiceLocator.h"
#include "Physics/Collision.h"
#include "Umbra.h"

namespace Umbra {
    void ContactResolver::ResolveContacts(Vector<Collision>& _collisions, double _deltaTime) {
        for (Collision collision : _collisions) {
            std::sort(collision.GetContacts().begin(), collision.GetContacts().end(),
                [](const ContactPoint& a, const ContactPoint& b) {
                    // Sort by penetration depth (deepest first)
                    // Priority 1: Most penetrating contacts
                    return (a.mPenetration > b.mPenetration);
                });
        }
        std::sort(_collisions.begin(), _collisions.end(), [](Collision& a, Collision& b) {
            // Sort by penetration depth (deepest first)
            // Priority 1: Most penetrating contacts
            return (a.GetContacts()[0].mPenetration > b.GetContacts()[0].mPenetration);
        });

        for (int iteration = 0; iteration < mMaxIteration; iteration++) {
            for (Collision collision : _collisions) {
                for (ContactPoint& contact : collision.GetContacts()) {

                    ServiceLocator::GetRenderService()->DebugDrawLine(contact.mContactPosition,
                        contact.mContactPosition + (contact.mContactNormal * contact.mPenetration), Color::Red);
                    ResolveContact(&contact, collision.mBodyA, collision.mBodyB, _deltaTime);
                }
            }
        }
    }

    void ContactResolver::ResolveContact(
        ContactPoint* _contact, EntityID _entityA, EntityID _entityB, double _deltaTime) {
        PhysicsBodyComponent* bodyA    = mBaseView->ecsRegister->GetComponent<PhysicsBodyComponent>(_entityA);
        PhysicsBodyComponent* bodyB    = mBaseView->ecsRegister->GetComponent<PhysicsBodyComponent>(_entityB);
        TransformComponent* transformA = mBaseView->ecsRegister->GetComponent<TransformComponent>(_entityA);
        TransformComponent* transformB = mBaseView->ecsRegister->GetComponent<TransformComponent>(_entityB);
        CalculateSeparatingVelocity(_contact, bodyA, bodyB, _deltaTime);
        ResolvePenetration(_contact, bodyA, transformA, bodyB, transformB);
    }

    void ContactResolver::CalculateSeparatingVelocity(ContactPoint* _contact, PhysicsBodyComponent* _physicsBodyA,
        PhysicsBodyComponent* _physicsBodyB, double _deltaTime) {

        /*
             velocity of object in the direction of the contact.
             if  < 0 object are moving towards each other
        */
        double separatingVelocity =
            Math::Vector2f::Dot((_physicsBodyA->mVelocity - _physicsBodyB->mVelocity), _contact->mContactNormal);
        if (separatingVelocity > 0) {
            // bodies already moving away from each other or are stationary
            // return;
        }
        // Calculate acceleration-induced velocity
        double velocityCausedViaAcceleration =
            Math::Vector2f::Dot((_physicsBodyA->mAcceleration - _physicsBodyB->mAcceleration), _contact->mContactNormal)
            * _deltaTime;


        // velocity required to stop further penetration + bounce
        // Remove acceleration-caused closing velocity
        double desiredDeltaVelocity = 0;
        double cofOfRestitution = 1; // Math::Min(_physicsBodyA->mCofOfRestitution, _physicsBodyB->mCofOfRestitution);
        if (velocityCausedViaAcceleration < 0) {
            // Case 1: Acceleration is pushing objects together
            desiredDeltaVelocity =
                -separatingVelocity * (1 + cofOfRestitution) + (velocityCausedViaAcceleration * cofOfRestitution);
            desiredDeltaVelocity = Math::Max(desiredDeltaVelocity, 0.0);
        } else {
            // Case 2: Acceleration isn't contributing to collision
            desiredDeltaVelocity = -separatingVelocity * (1 + cofOfRestitution);
        }

        double totalInverseMass = _physicsBodyA->mInverseMass + _physicsBodyB->mInverseMass;
        if (totalInverseMass <= 0) {
            // All bodies are static cannot apply impulse
            return;
        }
        // An impulse is the rate of change of the momentum, i.e. a force delivered in an instant
        //  impulse is force * Delta time
        //  since force = ma and acceleration is dv/dt
        //  impulse = m*dv ie dv/inverseMass
        //  impulse per unit of inverse mass.
        float impulseMagnitude = (float) (desiredDeltaVelocity / totalInverseMass);
        // impulse as vector
        Math::Vector2f impulsePerUnitInverseMass = _contact->mContactNormal * impulseMagnitude;
        _physicsBodyA->mVelocity += _physicsBodyA->mInverseMass * impulsePerUnitInverseMass;
        _physicsBodyB->mVelocity -= _physicsBodyB->mInverseMass * impulsePerUnitInverseMass;
    }

    void ContactResolver::ResolvePenetration(ContactPoint* _contact, PhysicsBodyComponent* _physicsBodyA,
        TransformComponent* _transformA, PhysicsBodyComponent* _physicsBodyB, TransformComponent* _transformB) {


        const float POSITION_SLOP = 0.01f; // Small allowed penetration

        if (_contact->mPenetration <= POSITION_SLOP) {
            //  we don’t have any penetration
            return;
        }
        double totalInverseMass = _physicsBodyA->mInverseMass + _physicsBodyB->mInverseMass;
        if (totalInverseMass <= 0) {
            // All bodies are static cannot move
            return;
        }
        Math::Vector2f displacementPerUnitInverseMass =
            _contact->mContactNormal * (_contact->mPenetration / totalInverseMass) * -1;
        _transformA->Position += displacementPerUnitInverseMass * _physicsBodyA->mInverseMass;
        _transformB->Position -= displacementPerUnitInverseMass * _physicsBodyB->mInverseMass;
    }
} // namespace Umbra
