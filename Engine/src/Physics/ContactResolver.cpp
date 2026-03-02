// #include "Physics/ContactResolver.h"
//
// #include "ECS/ECSRegister.h"
// #include "Service/ServiceLocator.h"
// #include "Physics/Collision.h"
// #include "Umbra.h"
//
// namespace Umbra {
//     void ContactResolver::ResolveContacts(Vector<Collision>& _collisions, double _deltaTime) {
//         for (Collision collision : _collisions) {
//             std::sort(collision.GetContacts().begin(), collision.GetContacts().end(),
//                 [](const ContactPoint& a, const ContactPoint& b) {
//                     return (a.mPenetration > b.mPenetration);
//                 });
//         }
//         std::sort(_collisions.begin(), _collisions.end(), [](Collision& a, Collision& b) {
//             return (a.GetContacts()[0].mPenetration > b.GetContacts()[0].mPenetration);
//         });
//
//         for (int iteration = 0; iteration < mMaxIteration; iteration++) {
//             for (Collision collision : _collisions) {
//                 for (ContactPoint& contact : collision.GetContacts()) {
//                     ServiceLocator::GetRenderService()->DebugDrawLine(contact.mContactPosition,
//                         contact.mContactPosition + (contact.mContactNormal * contact.mPenetration), Color::Red);
//                     ResolveContact(&contact, collision.mBodyA, collision.mBodyB, _deltaTime);
//                 }
//             }
//         }
//     }
//
//     void ContactResolver::ResolveContact(
//         ContactPoint* _contact, EntityID _entityA, EntityID _entityB, double _deltaTime) {
//         PhysicsBodyComponent* bodyA    = mBaseView->ecsRegister->GetComponent<PhysicsBodyComponent>(_entityA);
//         PhysicsBodyComponent* bodyB    = mBaseView->ecsRegister->GetComponent<PhysicsBodyComponent>(_entityB);
//         TransformComponent* transformA = mBaseView->ecsRegister->GetComponent<TransformComponent>(_entityA);
//         TransformComponent* transformB = mBaseView->ecsRegister->GetComponent<TransformComponent>(_entityB);
//         CalculateSeparatingVelocity(_contact, bodyA, bodyB, _deltaTime);
//         ResolvePenetration(_contact, bodyA, transformA, bodyB, transformB);
//     }
//
//     void ContactResolver::CalculateSeparatingVelocity(ContactPoint* _contact, PhysicsBodyComponent* _physicsBodyA,
//         PhysicsBodyComponent* _physicsBodyB, double _deltaTime) {
//         double separatingVelocity =
//             Math::Vector2f::Dot((_physicsBodyA->mVelocity - _physicsBodyB->mVelocity), _contact->mContactNormal);
//         if (separatingVelocity > 0) { }
//         double velocityCausedViaAcceleration =
//             Math::Vector2f::Dot((_physicsBodyA->mAcceleration - _physicsBodyB->mAcceleration), _contact->mContactNormal)
//             * _deltaTime;
//         double desiredDeltaVelocity = 0;
//         double cofOfRestitution = 1;
//         if (velocityCausedViaAcceleration < 0) {
//             desiredDeltaVelocity =
//                 -separatingVelocity * (1 + cofOfRestitution) + (velocityCausedViaAcceleration * cofOfRestitution);
//             desiredDeltaVelocity = Math::Max(desiredDeltaVelocity, 0.0);
//         } else {
//             desiredDeltaVelocity = -separatingVelocity * (1 + cofOfRestitution);
//         }
//         double totalInverseMass = _physicsBodyA->mInverseMass + _physicsBodyB->mInverseMass;
//         if (totalInverseMass <= 0) { return; }
//         float impulseMagnitude = (float)(desiredDeltaVelocity / totalInverseMass);
//         Math::Vector2f impulsePerUnitInverseMass = _contact->mContactNormal * impulseMagnitude;
//         _physicsBodyA->mVelocity += _physicsBodyA->mInverseMass * impulsePerUnitInverseMass;
//         _physicsBodyB->mVelocity -= _physicsBodyB->mInverseMass * impulsePerUnitInverseMass;
//     }
//
//     void ContactResolver::ResolvePenetration(ContactPoint* _contact, PhysicsBodyComponent* _physicsBodyA,
//         TransformComponent* _transformA, PhysicsBodyComponent* _physicsBodyB, TransformComponent* _transformB) {
//         const float POSITION_SLOP = 0.01f;
//         if (_contact->mPenetration <= POSITION_SLOP) { return; }
//         double totalInverseMass = _physicsBodyA->mInverseMass + _physicsBodyB->mInverseMass;
//         if (totalInverseMass <= 0) { return; }
//         Math::Vector2f displacementPerUnitInverseMass =
//             _contact->mContactNormal * (_contact->mPenetration / totalInverseMass) * -1;
//         _transformA->Position += displacementPerUnitInverseMass * _physicsBodyA->mInverseMass;
//         _transformB->Position -= displacementPerUnitInverseMass * _physicsBodyB->mInverseMass;
//     }
// } // namespace Umbra
