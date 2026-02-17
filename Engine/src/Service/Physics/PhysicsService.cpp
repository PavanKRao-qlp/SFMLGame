#include "Service/Physics/PhysicsService.h"

#include "Math/MathUtils.h"
#include "Service/Physics/CollisionQuery.h"

namespace Umbra {

    PhysicsService::PhysicsService() : PhysicsService(PhysicsServiceConfig{}) {}

    PhysicsService::PhysicsService(const PhysicsServiceConfig& _config) : mConfig(_config) {
        mBodies.reserve(_config.InitialBodyCapacity);
        mBroadphaseTree.SetFattenMargin(_config.AABBFattenMargin);
        mBroadphaseTree.SetDisplacementMultiplier(_config.AABBDisplacementMultiplier);
    }

    PhysicsService::~PhysicsService() {
        mBodies.clear();
        mFreeIndices.clear();
    }

    // ============== Configuration ==============

    void PhysicsService::SetGravity(Math::Vector2f _gravity) {
        mConfig.Gravity = _gravity;
    }

    Math::Vector2f PhysicsService::GetGravity() const {
        return mConfig.Gravity;
    }

    void PhysicsService::SetLinearDamping(float _damping) {
        mConfig.LinearDamping = _damping;
    }

    float PhysicsService::GetLinearDamping() const {
        return mConfig.LinearDamping;
    }

    void PhysicsService::SetAngularDamping(float _damping) {
        mConfig.AngularDamping = _damping;
    }

    float PhysicsService::GetAngularDamping() const {
        return mConfig.AngularDamping;
    }

    const PhysicsServiceConfig& PhysicsService::GetConfig() const {
        return mConfig;
    }

    PhysicsServiceConfig& PhysicsService::GetConfig() {
        return mConfig;
    }

    // ============== Simulation ==============

    void PhysicsService::Step(float _deltaTime) {
        // 0. Apply spring forces (force-based, before integration)
        ApplySpringForces(_deltaTime);

        // 1. Integrate forces -> accelerations -> velocities
        IntegrateForces(_deltaTime);

        // 1b. Save pre-integration positions for CCD bodies
        SaveCCDState();

        // 2. Integrate velocities -> positions
        IntegrateVelocities(_deltaTime);

        // 3. Apply damping
        ApplyDamping(_deltaTime);

        // 4. Clear force accumulators for next frame
        ClearForceAccumulators();

        // 4b. Sweep CCD bodies and clamp positions to earliest TOI
        PerformCCD();

        // 5. Update broadphase tree proxies
        UpdateBroadphaseProxies(_deltaTime);

        // 6. Check broad phase collisions
        BroadphaseDetection();

        // 7. Check Narrow phase collision for contacts
        NarrowPhaseDetection();

        // 8. Precompute constraint data (effective masses, bias) once per frame
        PrecomputeContactConstraints();

        // 8b. Precompute joint constraint solver cache
        PrecomputeConstraints(_deltaTime);

        // 9. Velocity solver - sequential impulse with accumulated clamping
        for (int i = 0; i < mConfig.VelocityIterations; i++) {
            ResolveContacts();
            SolveConstraintVelocities();
        }

        // 10. Position correction iterations (separate from velocity)
        for (int i = 0; i < mConfig.PositionIterations; i++) {
            PositionContraction();
            SolveConstraintPositions();
        }

        // 11. Sleep or Wake bodies
        UpdateSleepingBodies(_deltaTime);

        // 12. Categorize collision events (enter/stay/exit)
        CategorizeCollisionEvents();
    }

    void PhysicsService::IntegrateForces(float _deltaTime) {
        for (auto& body : mBodies) {
            if (!body.bIsActive || body.IsStatic() || body.bIsKinematic || body.bIsSleeping) {
                continue;
            }

            // Apply gravity
            if (body.bAffectedByGravity && body.InverseMass > 0) {
                Math::Vector2f gravityForce = mConfig.Gravity / body.InverseMass;
                body.ForceAccumulated += gravityForce;
            }

            // Calculate acceleration: a = F * inverseMass
            Math::Vector2f newAcceleration = body.ForceAccumulated * body.InverseMass;
            float newAngularAcceleration   = body.TorqueAccumulated * body.InverseInertia;

            // Velocity Verlet: v = v + 0.5 * (a_old + a_new) * dt
            body.Velocity += (body.Acceleration + newAcceleration) * 0.5f * _deltaTime;
            body.AngularVelocity += (body.AngularAcceleration + newAngularAcceleration) * 0.5f * _deltaTime;

            // Store acceleration for next frame
            body.Acceleration        = newAcceleration;
            body.AngularAcceleration = newAngularAcceleration;
        }
    }

    void PhysicsService::IntegrateVelocities(float _deltaTime) {
        for (auto& body : mBodies) {
            if (!body.bIsActive || body.IsStatic() || body.bIsKinematic || body.bIsSleeping) {
                continue;
            }

            // Position: p = p + v * dt + 0.5 * a * dt^2
            // Calculate displacement using s = vt + ((1/2) * at^2)
            body.Position += body.Velocity * _deltaTime + body.Acceleration * (Math::Pow(_deltaTime, 2) * 0.5f);

            // Angle: theta = theta + omega * dt + 0.5 * alpha * dt^2
            body.Angle +=
                body.AngularVelocity * _deltaTime + body.AngularAcceleration * (Math::Pow(_deltaTime, 2) * 0.5f);

            // Normalize angle to [0, 2*PI]
            body.Angle = Math::Fmod(body.Angle, Math::PI * 2.0f);
            if (body.Angle < 0) {
                body.Angle += Math::PI * 2.0f;
            }
        }
    }

    void PhysicsService::ApplyDamping(float _deltaTime) {
        for (auto& body : mBodies) {
            if (!body.bIsActive || body.IsStatic() || body.bIsKinematic || body.bIsSleeping) {
                continue;
            }

            // Exponential damping: v *= damping^dt
            float linearDamp  = body.LinearDamping > 0 ? body.LinearDamping : mConfig.LinearDamping;
            float angularDamp = body.AngularDamping > 0 ? body.AngularDamping : mConfig.AngularDamping;

            body.Velocity *= Math::Pow(linearDamp, _deltaTime);
            body.AngularVelocity *= Math::Pow(angularDamp, _deltaTime);
        }
    }

    void PhysicsService::ClearForceAccumulators() {
        for (auto& body : mBodies) {
            if (!body.bIsActive) {
                continue;
            }
            body.ForceAccumulated  = Math::Vector2f(0, 0);
            body.TorqueAccumulated = 0;
        }
    }


    void PhysicsService::SaveCCDState() {
        if (!mConfig.bEnableCCD) {
            return;
        }
        for (auto& body : mBodies) {
            if (!body.bIsActive || !body.bEnableCCD) {
                continue;
            }
            body.CCDSavedPosition = body.Position;
        }
    }

    void PhysicsService::PerformCCD() {
        if (!mConfig.bEnableCCD) {
            return;
        }

        for (uint32 i = 0; i < mBodies.size(); ++i) {
            auto& body = mBodies[i];
            if (!body.bIsActive || !body.bEnableCCD || body.IsStatic() || body.bIsSleeping) {
                continue;
            }

            Math::Vector2f displacement = body.Position - body.CCDSavedPosition;
            float displacementLen       = displacement.Magnitude();

            // Compute motion threshold relative to body extent
            float bodyExtent = 0.0f;
            if (body.BodyShape.IsCircle()) {
                bodyExtent = body.BodyShape.GetCircle().GetRadius() * 2.0f;
            } else if (body.BodyShape.IsBox()) {
                Math::Vector2f size = body.BodyShape.GetBox().GetSize();
                bodyExtent          = Math::Min(size.x, size.y);
            }
            float threshold = bodyExtent * mConfig.CCDMotionThreshold;

            if (displacementLen < threshold) {
                continue; // Motion too small to need CCD
            }

            // Build swept AABB (union of AABB at old and new position)
            body.UpdateBoundingAABB();
            Math::Bounds2D aabbOld(body.CCDSavedPosition, body.BoundingAABB.Size);
            Math::Bounds2D aabbNew(body.Position, body.BoundingAABB.Size);

            // Union of both AABBs
            Math::Vector2f sweepMin(
                Math::Min(aabbOld.Min().x, aabbNew.Min().x), Math::Min(aabbOld.Min().y, aabbNew.Min().y));
            Math::Vector2f sweepMax(
                Math::Max(aabbOld.Max().x, aabbNew.Max().x), Math::Max(aabbOld.Max().y, aabbNew.Max().y));
            Math::Vector2f sweepCenter = (sweepMin + sweepMax) * 0.5f;
            Math::Vector2f sweepSize   = sweepMax - sweepMin;
            Math::Bounds2D sweptAABB(sweepCenter, sweepSize);

            float minTOI = 1.0f;

            // Query broadphase for candidates
            mBroadphaseTree.Query(sweptAABB, [&](int32 _proxyId) {
                uint32 otherIndex = mBroadphaseTree.GetBodyIndex(_proxyId);
                if (otherIndex == i) {
                    return; // Skip self
                }
                if (otherIndex >= mBodies.size() || !mBodies[otherIndex].bIsActive) {
                    return;
                }
                if (!ShouldCollide(body, mBodies[otherIndex])) {
                    return;
                }

                float toi = CollisionQuery::ComputeTimeOfImpact(
                    body, body.CCDSavedPosition, mBodies[otherIndex], mConfig.CCDBoxBisectionIterations);

                if (toi < minTOI) {
                    minTOI = toi;
                }
            });

            // Clamp position to earliest TOI
            if (minTOI < 1.0f) {
                body.Position = body.CCDSavedPosition + displacement * minTOI;
            }
        }
    }

    void PhysicsService::UpdateBroadphaseProxies(float _deltaTime) {
        for (uint32 i = 0; i < mBodies.size(); ++i) {
            auto& body = mBodies[i];
            if (!body.bIsActive || body.TreeProxyId == NullNode) {
                continue;
            }

            body.UpdateBoundingAABB();
            Math::Bounds2D worldAABB(body.Position, body.BoundingAABB.Size);

            // Displacement prediction based on velocity
            Math::Vector2f displacement = body.Velocity * _deltaTime;
            mBroadphaseTree.MoveProxy(body.TreeProxyId, worldAABB, displacement);
        }
    }

    void PhysicsService::BroadphaseDetection() {
        mOverlappingBoundsIndexPair.clear();
        mBroadphaseChecks    = 0;
        mBroadphasePairCount = 0;

        if (mBodies.size() < 2) {
            return;
        }

        for (uint32 i = 0; i < mBodies.size(); ++i) {
            if (!mBodies[i].bIsActive || mBodies[i].TreeProxyId == NullNode) {
                continue;
            }

            const Math::Bounds2D& fatAABB = mBroadphaseTree.GetFatAABB(mBodies[i].TreeProxyId);

            mBroadphaseTree.Query(fatAABB, [&](int32 _proxyId) {
                ++mBroadphaseChecks;
                uint32 otherIndex = mBroadphaseTree.GetBodyIndex(_proxyId);

                // Skip self and deduplicate (only keep pairs where i < otherIndex)
                if (otherIndex <= i) {
                    return;
                }

                if (!mBodies[otherIndex].bIsActive) {
                    return;
                }

                // Skip pairs where both bodies are sleeping
                if (mBodies[i].bIsSleeping && mBodies[otherIndex].bIsSleeping) {
                    return;
                }

                // Skip pairs that fail collision filter
                if (!ShouldCollide(mBodies[i], mBodies[otherIndex])) {
                    return;
                }

                BodyHandle handleA;
                handleA.Index      = i;
                handleA.Generation = mBodies[i].Generation;
                BodyHandle handleB;
                handleB.Index      = otherIndex;
                handleB.Generation = mBodies[otherIndex].Generation;
                mOverlappingBoundsIndexPair.emplace_back(handleA, handleB);
                ++mBroadphasePairCount;
            });
        }
    }


    void PhysicsService::NarrowPhaseDetection() {
        // clear last frame Collision
        mCollisions.clear();
        for (const auto& handlePair : mOverlappingBoundsIndexPair) {
            CollisionDef collisionDef;
            collisionDef.handleA    = std::get<0>(handlePair);
            collisionDef.handleB    = std::get<1>(handlePair);
            PhysicsBodyData bodyA   = mBodies[collisionDef.handleA.Index];
            PhysicsBodyData bodyB   = mBodies[collisionDef.handleB.Index];
            collisionDef.bIsTrigger = bodyA.bIsTrigger || bodyB.bIsTrigger;
            if (CollisionQuery::CheckCollision(bodyA, bodyB, collisionDef)) {
                mCollisions.emplace_back(collisionDef);
            }
        }
    }

    void PhysicsService::PrecomputeContactConstraints() {
        for (CollisionDef& collision : mCollisions) {
            if (collision.bIsTrigger) {
                continue;
            }
            PhysicsBodyData& bodyA = mBodies[collision.handleA.Index];
            PhysicsBodyData& bodyB = mBodies[collision.handleB.Index];

            // Sleeping bodies are treated as immovable in the solver
            float invMassA    = bodyA.bIsSleeping ? 0.0f : bodyA.InverseMass;
            float invInertiaA = bodyA.bIsSleeping ? 0.0f : bodyA.InverseInertia;
            float invMassB    = bodyB.bIsSleeping ? 0.0f : bodyB.InverseMass;
            float invInertiaB = bodyB.bIsSleeping ? 0.0f : bodyB.InverseInertia;

            float invMassSum = invMassA + invMassB;
            float e          = Math::Min(bodyA.CoefOfRestitution, bodyB.CoefOfRestitution);

            // Fixed tangent direction perpendicular to contact normal
            Math::Vector2f tangent(-collision.contactNormal.y, collision.contactNormal.x);

            for (ContactDef& contact : collision.contacts) {
                // Lever arms from body centers to contact point
                contact.rA = contact.contactPoint - bodyA.Position;
                contact.rB = contact.contactPoint - bodyB.Position;

                // Effective mass along the normal:
                //   1 / (1/mA + 1/mB + (rA x n)^2/IA + (rB x n)^2/IB)
                float rACrossN     = Math::Vector2f::Cross2D(contact.rA, collision.contactNormal);
                float rBCrossN     = Math::Vector2f::Cross2D(contact.rB, collision.contactNormal);
                float normalDenom  = invMassSum + rACrossN * rACrossN * invInertiaA + rBCrossN * rBCrossN * invInertiaB;
                contact.normalMass = normalDenom > 0.0f ? 1.0f / normalDenom : 0.0f;

                // Effective mass along the tangent (same formula, tangent direction)
                float rACrossT     = Math::Vector2f::Cross2D(contact.rA, tangent);
                float rBCrossT     = Math::Vector2f::Cross2D(contact.rB, tangent);
                float tangentDenom = invMassSum + rACrossT * rACrossT * invInertiaA + rBCrossT * rBCrossT * invInertiaB;
                contact.tangentMass = tangentDenom > 0.0f ? 1.0f / tangentDenom : 0.0f;

                // Restitution velocity bias:
                // Only apply bounce if the closing speed is above a threshold (avoids jitter at rest)
                Math::Vector2f velA =
                    bodyA.Velocity + Math::Vector2f(-contact.rA.y, contact.rA.x) * bodyA.AngularVelocity;
                Math::Vector2f velB =
                    bodyB.Velocity + Math::Vector2f(-contact.rB.y, contact.rB.x) * bodyB.AngularVelocity;
                float closingSpeed   = Math::Vector2f::Dot(velB - velA, collision.contactNormal);
                contact.velocityBias = closingSpeed < -1.0f ? -e * closingSpeed : 0.0f;

                // Reset accumulators for this frame
                contact.normalImpulseAccum  = 0.0f;
                contact.tangentImpulseAccum = 0.0f;
            }
        }
    }

    void PhysicsService::ResolveContacts() {
        for (CollisionDef& collision : mCollisions) {
            if (collision.bIsTrigger) {
                continue;
            }
            PhysicsBodyData& bodyA = mBodies[collision.handleA.Index];
            PhysicsBodyData& bodyB = mBodies[collision.handleB.Index];

            if (bodyA.IsStatic() && bodyB.IsStatic()) {
                continue;
            }

            // Skip pairs where both bodies are sleeping
            if (bodyA.bIsSleeping && bodyB.bIsSleeping) {
                continue;
            }

            // Sleeping bodies are treated as immovable in the solver
            float invMassA    = bodyA.bIsSleeping ? 0.0f : bodyA.InverseMass;
            float invInertiaA = bodyA.bIsSleeping ? 0.0f : bodyA.InverseInertia;
            float invMassB    = bodyB.bIsSleeping ? 0.0f : bodyB.InverseMass;
            float invInertiaB = bodyB.bIsSleeping ? 0.0f : bodyB.InverseInertia;

            // Fixed tangent perpendicular to contact normal
            Math::Vector2f tangent(-collision.contactNormal.y, collision.contactNormal.x);

            // Friction coefficient (geometric mean of both bodies)
            float friction = Math::Sqrt(bodyA.DynamicFriction * bodyB.DynamicFriction);

            for (ContactDef& contact : collision.contacts) {
                // === NORMAL IMPULSE with accumulated clamping ===

                // Current relative velocity at contact point
                Math::Vector2f velA =
                    bodyA.Velocity + Math::Vector2f(-contact.rA.y, contact.rA.x) * bodyA.AngularVelocity;
                Math::Vector2f velB =
                    bodyB.Velocity + Math::Vector2f(-contact.rB.y, contact.rB.x) * bodyB.AngularVelocity;
                Math::Vector2f relVel = velB - velA;

                // Compute delta impulse: dj = (-v_rel.n + bias) * effectiveMass
                float velAlongNormal = Math::Vector2f::Dot(relVel, collision.contactNormal);
                float dj             = (-velAlongNormal + contact.velocityBias) * contact.normalMass;

                // Accumulate and clamp: total normal impulse must be >= 0 (can only push, never pull)
                float oldNormalAccum       = contact.normalImpulseAccum;
                contact.normalImpulseAccum = Math::Max(oldNormalAccum + dj, 0.0f);
                dj                         = contact.normalImpulseAccum - oldNormalAccum;

                // Apply the delta impulse directly to bodies
                Math::Vector2f normalImpulse = collision.contactNormal * dj;
                bodyA.Velocity -= normalImpulse * invMassA;
                bodyA.AngularVelocity -= Math::Vector2f::Cross2D(contact.rA, normalImpulse) * invInertiaA;
                bodyB.Velocity += normalImpulse * invMassB;
                bodyB.AngularVelocity += Math::Vector2f::Cross2D(contact.rB, normalImpulse) * invInertiaB;

                // === FRICTION IMPULSE with accumulated Coulomb clamping ===

                // Recompute relative velocity after normal impulse changed velocities
                velA   = bodyA.Velocity + Math::Vector2f(-contact.rA.y, contact.rA.x) * bodyA.AngularVelocity;
                velB   = bodyB.Velocity + Math::Vector2f(-contact.rB.y, contact.rB.x) * bodyB.AngularVelocity;
                relVel = velB - velA;

                // Delta friction impulse along fixed tangent
                float velAlongTangent = Math::Vector2f::Dot(relVel, tangent);
                float djt             = -velAlongTangent * contact.tangentMass;

                // Coulomb clamp: |friction impulse| <= mu * normal impulse
                float maxFriction           = friction * contact.normalImpulseAccum;
                float oldTangentAccum       = contact.tangentImpulseAccum;
                contact.tangentImpulseAccum = Math::Clamp(oldTangentAccum + djt, -maxFriction, maxFriction);
                djt                         = contact.tangentImpulseAccum - oldTangentAccum;

                // Apply the delta friction impulse
                Math::Vector2f frictionImpulse = tangent * djt;
                bodyA.Velocity -= frictionImpulse * invMassA;
                bodyA.AngularVelocity -= Math::Vector2f::Cross2D(contact.rA, frictionImpulse) * invInertiaA;
                bodyB.Velocity += frictionImpulse * invMassB;
                bodyB.AngularVelocity += Math::Vector2f::Cross2D(contact.rB, frictionImpulse) * invInertiaB;
            }
        }
    }

    void PhysicsService::PositionContraction() {
        for (const CollisionDef& collisionDef : mCollisions) {
            if (collisionDef.bIsTrigger) {
                continue;
            }
            PhysicsBodyData& bodyA = *GetBodyData(collisionDef.handleA);
            PhysicsBodyData& bodyB = *GetBodyData(collisionDef.handleB);

            // Skip if both bodies are static
            if (bodyA.IsStatic() && bodyB.IsStatic()) {
                continue;
            }

            // Skip pairs where both bodies are sleeping
            if (bodyA.bIsSleeping && bodyB.bIsSleeping) {
                continue;
            }

            // Correction parameters
            const float percent = 0.4f; // Correction percentage (0.2 to 0.8)
            const float slop    = 0.01f; // Penetration allowance to avoid jitter

            // Only correct if penetration exceeds slop
            float correctionMag = Math::Max(collisionDef.penetration - slop, 0.0f);
            if (correctionMag <= 0.0f) {
                continue;
            }

            // Sleeping bodies are treated as immovable
            float invMassA = bodyA.bIsSleeping ? 0.0f : bodyA.InverseMass;
            float invMassB = bodyB.bIsSleeping ? 0.0f : bodyB.InverseMass;

            // Calculate total inverse mass
            float invMassSum = invMassA + invMassB;
            if (invMassSum <= 0.0f) {
                return; // Both have infinite mass
            }

            // Correction vector along the contact normal
            Math::Vector2f correction = collisionDef.contactNormal * (correctionMag / invMassSum) * percent;

            // Move bodies apart proportional to their inverse mass
            // Heavier objects move less, lighter objects move more
            bodyA.Position -= correction * invMassA;
            bodyB.Position += correction * invMassB;
        }
    }


    void PhysicsService::UpdateSleepingBodies(float _deltaTime) {
        float linSleepSq = mConfig.LinearVelocitySleepThreshold * mConfig.LinearVelocitySleepThreshold;
        float angSleep   = mConfig.AngularVelocitySleepThreshold;

        // Returns true if a body's velocity is below the sleep thresholds
        auto isBelowSleepThreshold = [&](const PhysicsBodyData& _body) -> bool {
            return Math::Vector2f::Dot(_body.Velocity, _body.Velocity) < linSleepSq
                && Math::Abs(_body.AngularVelocity) < angSleep;
        };

        // A sleeping body should wake only if it's colliding with a significantly moving body
        for (const CollisionDef& collisionDef : mCollisions) {
            if (collisionDef.bIsTrigger) {
                continue;
            }
            PhysicsBodyData& bodyA = *GetBodyData(collisionDef.handleA);
            PhysicsBodyData& bodyB = *GetBodyData(collisionDef.handleB);

            // Skip if both bodies are static
            if (bodyA.IsStatic() && bodyB.IsStatic()) {
                continue;
            }
            // Only wake the sleeper if the awake partner is moving above sleep threshold
            if (bodyA.bIsSleeping && !bodyB.bIsSleeping && !bodyB.IsStatic()) {
                if (!isBelowSleepThreshold(bodyB)) {
                    bodyA.Wake();
                }
            }
            if (bodyB.bIsSleeping && !bodyA.bIsSleeping && !bodyA.IsStatic()) {
                if (!isBelowSleepThreshold(bodyA)) {
                    bodyB.Wake();
                }
            }
        }

        // Wake constraint partners: only if the awake partner is moving significantly
        auto wakePartner = [&](const BodyHandle& _hA, const BodyHandle& _hB) {
            if (!IsBodyValid(_hA) || !IsBodyValid(_hB)) {
                return;
            }
            PhysicsBodyData& bA = mBodies[_hA.Index];
            PhysicsBodyData& bB = mBodies[_hB.Index];
            if (bA.bIsSleeping && !bB.bIsSleeping && !bB.IsStatic()) {
                if (!isBelowSleepThreshold(bB)) {
                    bA.Wake();
                }
            }
            if (bB.bIsSleeping && !bA.bIsSleeping && !bA.IsStatic()) {
                if (!isBelowSleepThreshold(bA)) {
                    bB.Wake();
                }
            }
        };
        for (const auto& c : mDistanceConstraints) {
            if (c.bIsActive) {
                wakePartner(c.HandleA, c.HandleB);
            }
        }
        for (const auto& c : mHingeConstraints) {
            if (c.bIsActive) {
                wakePartner(c.HandleA, c.HandleB);
            }
        }
        // Update sleep timers and put bodies to sleep
        for (auto& body : mBodies) {
            if (body.IsStatic() || !body.bCanSleep) {
                continue;
            }
            // Already sleeping, skip
            if (body.bIsSleeping) {
                continue;
            }
            // Check if below sleep threshold
            bool bShouldSleep = body.Velocity.SquareMagnitude() < Math::Pow(mConfig.LinearVelocitySleepThreshold, 2)
                             && Math::Abs(body.AngularVelocity) < mConfig.AngularVelocitySleepThreshold;

            if (bShouldSleep) {
                body.SleepTimer += _deltaTime;
                if (body.SleepTimer >= mConfig.SleepTimeThreshold) {
                    body.Sleep();
                }
            } else {
                // Reset timer if moving
                body.SleepTimer = 0.0f;
            }
        }
    }

    // ============== Broadphase Access ==============

    const DynamicAABBTree& PhysicsService::GetBroadphaseTree() const {
        return mBroadphaseTree;
    }

    uint32 PhysicsService::GetBroadphaseChecks() const {
        return mBroadphaseChecks;
    }

    uint32 PhysicsService::GetBroadphasePairCount() const {
        return mBroadphasePairCount;
    }

    // ============== Body Management ==============

    BodyHandle PhysicsService::CreateBody(const BodyDef& _def) {
        uint32 index;
        uint32 generation;

        if (!mFreeIndices.empty()) {
            // Reuse a free slot
            index = mFreeIndices.back();
            mFreeIndices.pop_back();
            generation = mBodies[index].Generation + 1;
        } else {
            // Allocate new slot
            index      = static_cast<uint32>(mBodies.size());
            generation = 0;
            mBodies.emplace_back();
        }

        PhysicsBodyData& body    = mBodies[index];
        body.Position            = _def.Position;
        body.Angle               = Math::DegreeToRadian(_def.Angle);
        body.Velocity            = _def.Velocity;
        body.AngularVelocity     = _def.AngularVelocity;
        body.Acceleration        = Math::Vector2f(0, 0);
        body.AngularAcceleration = 0;
        body.ForceAccumulated    = Math::Vector2f(0, 0);
        body.TorqueAccumulated   = 0;
        body.LinearDamping       = _def.LinearDamping;
        body.AngularDamping      = _def.AngularDamping;
        body.BodyShape           = _def.ShapeData;
        body.CoefOfRestitution   = _def.CoefOfRestitution;
        body.StaticFriction      = _def.StaticFriction;
        body.DynamicFriction     = _def.DynamicFriction;
        body.bAffectedByGravity  = _def.bAffectedByGravity;
        body.bIsKinematic        = _def.bIsKinematic;
        body.bCanSleep           = _def.bCanSleep;
        body.bIsTrigger          = _def.bIsTrigger;
        body.bEnableCCD          = _def.bEnableCCD;
        body.Filter              = _def.Filter;
        body.bIsActive           = true;
        body.UserData            = _def.UserData;
        body.Generation          = generation;

        // Set mass
        body.SetMass(_def.Mass);

        // Calculate inertia from shape if not explicitly provided
        float inertia = _def.Inertia;
        if (inertia <= 0.0f && _def.Mass > 0.0f) {
            if (_def.ShapeData.IsCircle()) {
                // Solid disk: I = (1/2) * m * r^2
                float r = _def.ShapeData.GetCircle().GetRadius();
                inertia = 0.5f * _def.Mass * r * r;
            } else if (_def.ShapeData.IsBox()) {
                // Solid rectangle: I = (1/12) * m * (w^2 + h^2)
                float w = _def.ShapeData.GetBox().GetWidth();
                float h = _def.ShapeData.GetBox().GetHeight();
                inertia = (1.0f / 12.0f) * _def.Mass * (w * w + h * h);
            }
        }
        body.SetInertia(inertia);

        // Compute bounding AABB and insert into broadphase tree
        body.UpdateBoundingAABB();
        Math::Bounds2D worldAABB(body.Position, body.BoundingAABB.Size);
        body.TreeProxyId = mBroadphaseTree.InsertProxy(worldAABB, index);

        ++mActiveBodyCount;

        BodyHandle handle;
        handle.Index      = index;
        handle.Generation = generation;
        return handle;
    }

    void PhysicsService::DestroyBody(BodyHandle _handle) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body == nullptr) {
            return;
        }

        // Destroy any constraints referencing this body
        DestroyConstraintsForBody(_handle.Index);

        // Remove from broadphase tree before deactivating
        if (body->TreeProxyId != NullNode) {
            mBroadphaseTree.RemoveProxy(body->TreeProxyId);
            body->TreeProxyId = NullNode;
        }

        body->bIsActive = false;
        body->UserData  = nullptr;
        mFreeIndices.push_back(_handle.Index);
        --mActiveBodyCount;
    }

    bool PhysicsService::IsBodyValid(BodyHandle _handle) const {
        if (!_handle.IsValid() || _handle.Index >= mBodies.size()) {
            return false;
        }
        const PhysicsBodyData& body = mBodies[_handle.Index];
        return body.bIsActive && body.Generation == _handle.Generation;
    }

    uint32 PhysicsService::GetBodyCount() const {
        return mActiveBodyCount;
    }

    // ============== Body State (Read) ==============

    Math::Vector2f PhysicsService::GetPosition(BodyHandle _handle) const {
        const PhysicsBodyData* body = GetBodyDataInternal(_handle);
        return body ? body->Position : Math::Vector2f(0, 0);
    }

    float PhysicsService::GetAngle(BodyHandle _handle) const {
        const PhysicsBodyData* body = GetBodyDataInternal(_handle);
        return body ? Math::RadianToDegree(body->Angle) : 0.0f;
    }

    Math::Vector2f PhysicsService::GetVelocity(BodyHandle _handle) const {
        const PhysicsBodyData* body = GetBodyDataInternal(_handle);
        return body ? body->Velocity : Math::Vector2f(0, 0);
    }

    float PhysicsService::GetAngularVelocity(BodyHandle _handle) const {
        const PhysicsBodyData* body = GetBodyDataInternal(_handle);
        return body ? body->AngularVelocity : 0.0f;
    }

    Math::Vector2f PhysicsService::GetAcceleration(BodyHandle _handle) const {
        const PhysicsBodyData* body = GetBodyDataInternal(_handle);
        return body ? body->Acceleration : Math::Vector2f(0, 0);
    }

    float PhysicsService::GetAngularAcceleration(BodyHandle _handle) const {
        const PhysicsBodyData* body = GetBodyDataInternal(_handle);
        return body ? body->AngularAcceleration : 0.0f;
    }

    float PhysicsService::GetMass(BodyHandle _handle) const {
        const PhysicsBodyData* body = GetBodyDataInternal(_handle);
        return body && body->InverseMass > 0.0f ? 1.0f / body->InverseMass : 0.0f;
    }

    float PhysicsService::GetInertia(BodyHandle _handle) const {
        const PhysicsBodyData* body = GetBodyDataInternal(_handle);
        return body && body->InverseInertia > 0.0f ? 1.0f / body->InverseInertia : 0.0f;
    }

    void* PhysicsService::GetUserData(BodyHandle _handle) const {
        const PhysicsBodyData* body = GetBodyDataInternal(_handle);
        return body ? body->UserData : nullptr;
    }

    bool PhysicsService::IsKinematic(BodyHandle _handle) const {
        const PhysicsBodyData* body = GetBodyDataInternal(_handle);
        return body ? body->bIsKinematic : false;
    }

    bool PhysicsService::IsStatic(BodyHandle _handle) const {
        const PhysicsBodyData* body = GetBodyDataInternal(_handle);
        return body ? body->IsStatic() : true;
    }

    // ============== Body State (Write) ==============

    void PhysicsService::SetPosition(BodyHandle _handle, Math::Vector2f _position) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body) {
            body->Position = _position;
        }
    }

    void PhysicsService::SetAngle(BodyHandle _handle, float _angle) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body) {
            body->Angle = Math::DegreeToRadian(_angle);
        }
    }

    void PhysicsService::SetVelocity(BodyHandle _handle, Math::Vector2f _velocity) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body) {
            body->Velocity = _velocity;
        }
    }

    void PhysicsService::SetAngularVelocity(BodyHandle _handle, float _angularVelocity) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body) {
            body->AngularVelocity = _angularVelocity;
        }
    }

    void PhysicsService::SetMass(BodyHandle _handle, float _mass) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body) {
            body->SetMass(_mass);
        }
    }

    void PhysicsService::SetInertia(BodyHandle _handle, float _inertia) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body) {
            body->SetInertia(_inertia);
        }
    }

    void PhysicsService::SetUserData(BodyHandle _handle, void* _userData) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body) {
            body->UserData = _userData;
        }
    }

    void PhysicsService::SetKinematic(BodyHandle _handle, bool _bIsKinematic) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body) {
            body->bIsKinematic = _bIsKinematic;
        }
    }

    void PhysicsService::SetAffectedByGravity(BodyHandle _handle, bool _bAffected) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body) {
            body->bAffectedByGravity = _bAffected;
        }
    }

    void PhysicsService::SetCoefOfRestitution(BodyHandle _handle, float _coefOfRestitution) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body) {
            body->CoefOfRestitution = _coefOfRestitution;
        }
    }

    float PhysicsService::GetCoefOfRestitution(BodyHandle _handle) const {
        const PhysicsBodyData* body = GetBodyDataInternal(_handle);
        return body ? body->CoefOfRestitution : 1.0f;
    }

    void PhysicsService::SetStaticFriction(BodyHandle _handle, float _staticFriction) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body) {
            body->StaticFriction = _staticFriction;
        }
    }

    float PhysicsService::GetStaticFriction(BodyHandle _handle) const {
        const PhysicsBodyData* body = GetBodyDataInternal(_handle);
        return body ? body->StaticFriction : 0.6f;
    }

    void PhysicsService::SetDynamicFriction(BodyHandle _handle, float _dynamicFriction) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body) {
            body->DynamicFriction = _dynamicFriction;
        }
    }

    float PhysicsService::GetDynamicFriction(BodyHandle _handle) const {
        const PhysicsBodyData* body = GetBodyDataInternal(_handle);
        return body ? body->DynamicFriction : 0.4f;
    }

    CollisionFilter PhysicsService::GetCollisionFilter(BodyHandle _handle) const {
        const PhysicsBodyData* body = GetBodyDataInternal(_handle);
        return body ? body->Filter : CollisionFilter{};
    }

    void PhysicsService::SetCollisionFilter(BodyHandle _handle, const CollisionFilter& _filter) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body) {
            body->Filter = _filter;
        }
    }

    bool PhysicsService::IsTrigger(BodyHandle _handle) const {
        const PhysicsBodyData* body = GetBodyDataInternal(_handle);
        return body ? body->bIsTrigger : false;
    }

    void PhysicsService::SetTrigger(BodyHandle _handle, bool _bIsTrigger) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body) {
            body->bIsTrigger = _bIsTrigger;
        }
    }

    bool PhysicsService::IsCCDEnabled(BodyHandle _handle) const {
        const PhysicsBodyData* body = GetBodyDataInternal(_handle);
        return body ? body->bEnableCCD : false;
    }

    void PhysicsService::SetCCDEnabled(BodyHandle _handle, bool _bEnable) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body) {
            body->bEnableCCD = _bEnable;
        }
    }

    // ============== Force Application ==============

    void PhysicsService::ApplyForce(BodyHandle _handle, Math::Vector2f _force) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body && !body->IsStatic() && !body->bIsKinematic) {
            if (_force.SquareMagnitude() > 0.0f) {
                body->Wake();
            }
            body->ForceAccumulated += _force;
        }
    }

    void PhysicsService::ApplyForceAtPoint(BodyHandle _handle, Math::Vector2f _force, Math::Vector2f _worldPoint) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body && !body->IsStatic() && !body->bIsKinematic) {
            if (_force.SquareMagnitude() > 0.0f) {
                body->Wake();
            }
            body->ForceAccumulated += _force;
            Math::Vector2f r = _worldPoint - body->Position;
            float torque     = Math::Vector2f::Cross2D(r, _force);
            body->TorqueAccumulated += torque;
        }
    }

    void PhysicsService::ApplyForceAtLocalPoint(BodyHandle _handle, Math::Vector2f _force, Math::Vector2f _localPoint) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body && !body->IsStatic() && !body->bIsKinematic) {
            if (_force.SquareMagnitude() > 0.0f) {
                body->Wake();
            }
            body->ForceAccumulated += _force;
            Math::Vector2f rotatedPoint = _localPoint.GetRotated(body->Angle);
            float torque                = Math::Vector2f::Cross2D(rotatedPoint, _force);
            body->TorqueAccumulated += torque;
        }
    }

    void PhysicsService::ApplyImpulse(BodyHandle _handle, Math::Vector2f _impulse) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body && !body->IsStatic() && !body->bIsKinematic) {
            if (_impulse.SquareMagnitude() > 0.0f) {
                body->Wake();
            }
            body->Velocity += _impulse * body->InverseMass;
        }
    }

    void PhysicsService::ApplyImpulseAtPoint(
        BodyHandle _handle, Math::Vector2f _impulse, Math::Vector2f _worldPoint, bool _bShouldAwake) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body && !body->IsStatic() && !body->bIsKinematic) {
            if (_bShouldAwake && _impulse.SquareMagnitude() > 0.0f) {
                body->Wake();
            }
            body->Velocity += _impulse * body->InverseMass;
            Math::Vector2f r     = _worldPoint - body->Position;
            float angularImpulse = Math::Vector2f::Cross2D(r, _impulse);
            body->AngularVelocity += angularImpulse * body->InverseInertia;
        }
    }

    void PhysicsService::ApplyTorque(BodyHandle _handle, float _torque) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body && !body->IsStatic() && !body->bIsKinematic) {
            if (Math::Abs(_torque) > 0.0f) {
                body->Wake();
            }
            body->TorqueAccumulated += _torque;
        }
    }

    void PhysicsService::ApplyAngularImpulse(BodyHandle _handle, float _impulse) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body && !body->IsStatic() && !body->bIsKinematic) {
            if (Math::Abs(_impulse) > 0.0f) {
                body->Wake();
            }
            body->AngularVelocity += _impulse * body->InverseInertia;
        }
    }

    // ============== Collision Queries ==============

    bool PhysicsService::TestOverlap(BodyHandle _a, BodyHandle _b) const {
        const PhysicsBodyData* bodyA = GetBodyDataInternal(_a);
        const PhysicsBodyData* bodyB = GetBodyDataInternal(_b);
        if (bodyA == nullptr || bodyB == nullptr) {
            return false;
        }
        return false; // CollisionQuery::TestOverlap(bodyA->BodyShape, bodyA->Position, bodyB->BodyShape,
                      // bodyB->Position);
    }

    const Vector<CollisionDef>& PhysicsService::GetCollisions() const {
        return mCollisions;
    }

    const Vector<CollisionEvent>& PhysicsService::GetCollisionEnterEvents() const {
        return mCollisionEnterEvents;
    }

    const Vector<CollisionEvent>& PhysicsService::GetCollisionStayEvents() const {
        return mCollisionStayEvents;
    }

    const Vector<CollisionEvent>& PhysicsService::GetCollisionExitEvents() const {
        return mCollisionExitEvents;
    }

    const Vector<CollisionEvent>& PhysicsService::GetTriggerEnterEvents() const {
        return mTriggerEnterEvents;
    }

    const Vector<CollisionEvent>& PhysicsService::GetTriggerStayEvents() const {
        return mTriggerStayEvents;
    }

    const Vector<CollisionEvent>& PhysicsService::GetTriggerExitEvents() const {
        return mTriggerExitEvents;
    }

    uint64 PhysicsService::MakeCollisionPairKey(uint32 _indexA, uint32 _indexB) {
        uint32 lo = _indexA < _indexB ? _indexA : _indexB;
        uint32 hi = _indexA < _indexB ? _indexB : _indexA;
        return (static_cast<uint64>(lo) << 32) | static_cast<uint64>(hi);
    }

    bool PhysicsService::ShouldCollide(const PhysicsBodyData& _a, const PhysicsBodyData& _b) {
        return (_a.Filter.CategoryBits & _b.Filter.MaskBits) != 0 && (_b.Filter.CategoryBits & _a.Filter.MaskBits) != 0;
    }

    void PhysicsService::CategorizeCollisionEvents() {
        mCollisionEnterEvents.clear();
        mCollisionStayEvents.clear();
        mCollisionExitEvents.clear();
        mTriggerEnterEvents.clear();
        mTriggerStayEvents.clear();
        mTriggerExitEvents.clear();

        Set<uint64> currentCollisionPairs;
        Set<uint64> currentTriggerPairs;

        for (const CollisionDef& collision : mCollisions) {
            uint64 key = MakeCollisionPairKey(collision.handleA.Index, collision.handleB.Index);

            CollisionEvent event;
            event.HandleA       = collision.handleA;
            event.HandleB       = collision.handleB;
            event.ContactNormal = collision.contactNormal;
            event.Penetration   = collision.penetration;
            event.bIsTrigger    = collision.bIsTrigger;

            if (collision.bIsTrigger) {
                currentTriggerPairs.insert(key);
                if (mPreviousTriggerPairs.find(key) != mPreviousTriggerPairs.end()) {
                    mTriggerStayEvents.emplace_back(event);
                } else {
                    mTriggerEnterEvents.emplace_back(event);
                }
            } else {
                currentCollisionPairs.insert(key);
                if (mPreviousCollisionPairs.find(key) != mPreviousCollisionPairs.end()) {
                    mCollisionStayEvents.emplace_back(event);
                } else {
                    mCollisionEnterEvents.emplace_back(event);
                }
            }
        }

        // Collision pairs that were active last frame but not this frame → Exit
        for (uint64 prevKey : mPreviousCollisionPairs) {
            if (currentCollisionPairs.find(prevKey) == currentCollisionPairs.end()) {
                CollisionEvent event;
                event.HandleA.Index = static_cast<uint32>(prevKey >> 32);
                event.HandleB.Index = static_cast<uint32>(prevKey & 0xFFFFFFFF);
                if (event.HandleA.Index < mBodies.size() && mBodies[event.HandleA.Index].bIsActive) {
                    event.HandleA.Generation = mBodies[event.HandleA.Index].Generation;
                }
                if (event.HandleB.Index < mBodies.size() && mBodies[event.HandleB.Index].bIsActive) {
                    event.HandleB.Generation = mBodies[event.HandleB.Index].Generation;
                }
                event.ContactNormal = Math::Vector2f(0, 0);
                event.Penetration   = 0.0f;
                mCollisionExitEvents.emplace_back(event);
            }
        }

        // Trigger pairs that were active last frame but not this frame → Exit
        for (uint64 prevKey : mPreviousTriggerPairs) {
            if (currentTriggerPairs.find(prevKey) == currentTriggerPairs.end()) {
                CollisionEvent event;
                event.HandleA.Index = static_cast<uint32>(prevKey >> 32);
                event.HandleB.Index = static_cast<uint32>(prevKey & 0xFFFFFFFF);
                if (event.HandleA.Index < mBodies.size() && mBodies[event.HandleA.Index].bIsActive) {
                    event.HandleA.Generation = mBodies[event.HandleA.Index].Generation;
                }
                if (event.HandleB.Index < mBodies.size() && mBodies[event.HandleB.Index].bIsActive) {
                    event.HandleB.Generation = mBodies[event.HandleB.Index].Generation;
                }
                event.ContactNormal = Math::Vector2f(0, 0);
                event.Penetration   = 0.0f;
                event.bIsTrigger    = true;
                mTriggerExitEvents.emplace_back(event);
            }
        }

        mPreviousCollisionPairs = std::move(currentCollisionPairs);
        mPreviousTriggerPairs   = std::move(currentTriggerPairs);
    }

    // ============== Internal Access ==============

    PhysicsBodyData* PhysicsService::GetBodyData(BodyHandle _handle) {
        return GetBodyDataInternal(_handle);
    }

    const PhysicsBodyData* PhysicsService::GetBodyData(BodyHandle _handle) const {
        return GetBodyDataInternal(_handle);
    }

    BodyHandle PhysicsService::GetBodyHandle(const PhysicsBodyData& _bodyData) const {
        const PhysicsBodyData* begin = mBodies.data();
        const PhysicsBodyData* end   = begin + mBodies.size();

        if (&_bodyData < begin || &_bodyData >= end) {
            return BodyHandle::Invalid();
        }

        uint32 index = static_cast<uint32>(&_bodyData - begin);
        if (!mBodies[index].bIsActive) {
            return BodyHandle::Invalid();
        }

        BodyHandle handle;
        handle.Index      = index;
        handle.Generation = mBodies[index].Generation;
        return handle;
    }

    PhysicsBodyData* PhysicsService::GetBodyDataInternal(BodyHandle _handle) {
        if (!IsBodyValid(_handle)) {
            return nullptr;
        }
        return &mBodies[_handle.Index];
    }

    const PhysicsBodyData* PhysicsService::GetBodyDataInternal(BodyHandle _handle) const {
        if (!IsBodyValid(_handle)) {
            return nullptr;
        }
        return &mBodies[_handle.Index];
    }

    // ============== Spatial Queries ==============

    bool PhysicsService::PointInBody(Math::Vector2f _point, const PhysicsBodyData& _body) const {
        if (_body.BodyShape.IsCircle()) {
            float r  = _body.BodyShape.GetCircle().GetRadius();
            float dx = _point.x - _body.Position.x;
            float dy = _point.y - _body.Position.y;
            return (dx * dx + dy * dy) <= (r * r);
        }

        if (_body.BodyShape.IsBox()) {
            // Transform point into body-local space (undo rotation)
            float dx     = _point.x - _body.Position.x;
            float dy     = _point.y - _body.Position.y;
            float cosA   = Math::Cos(-_body.Angle);
            float sinA   = Math::Sin(-_body.Angle);
            float localX = dx * cosA - dy * sinA;
            float localY = dx * sinA + dy * cosA;

            Math::Vector2f halfSize = _body.BodyShape.GetBox().GetSize() * 0.5f;
            return Math::Abs(localX) <= halfSize.x && Math::Abs(localY) <= halfSize.y;
        }

        return false;
    }

    bool PhysicsService::RaycastBody(Math::Vector2f _origin, Math::Vector2f _direction, float _maxDistance,
        const PhysicsBodyData& _body, float& _outDistance, Math::Vector2f& _outNormal) const {

        if (_body.BodyShape.IsCircle()) {
            float r  = _body.BodyShape.GetCircle().GetRadius();
            float dx = _origin.x - _body.Position.x;
            float dy = _origin.y - _body.Position.y;

            float a = Math::Vector2f::Dot(_direction, _direction);
            Math::Vector2f d(dx, dy);
            float b    = 2.0f * Math::Vector2f::Dot(d, _direction);
            float c    = Math::Vector2f::Dot(d, d) - r * r;
            float disc = b * b - 4.0f * a * c;

            if (disc < 0.0f) {
                return false;
            }

            float sqrtDisc = Math::Sqrt(disc);
            float t        = (-b - sqrtDisc) / (2.0f * a);

            // If the near root is behind us, try the far root (origin inside circle)
            if (t < 0.0f) {
                t = (-b + sqrtDisc) / (2.0f * a);
            }

            if (t < 0.0f || t > _maxDistance) {
                return false;
            }

            _outDistance            = t;
            Math::Vector2f hitPoint = _origin + _direction * t;
            _outNormal              = hitPoint - _body.Position;
            _outNormal.Normalize();
            return true;
        }

        if (_body.BodyShape.IsBox()) {
            // Transform ray into body-local space
            float cosA = Math::Cos(-_body.Angle);
            float sinA = Math::Sin(-_body.Angle);

            float odx = _origin.x - _body.Position.x;
            float ody = _origin.y - _body.Position.y;
            Math::Vector2f localOrigin(odx * cosA - ody * sinA, odx * sinA + ody * cosA);
            Math::Vector2f localDir(
                _direction.x * cosA - _direction.y * sinA, _direction.x * sinA + _direction.y * cosA);

            Math::Vector2f halfSize = _body.BodyShape.GetBox().GetSize() * 0.5f;

            // Slab intersection in local space
            float tMin = 0.0f;
            float tMax = _maxDistance;
            Math::Vector2f localNormal(0, 0);

            // X slab
            if (Math::Abs(localDir.x) < Math::EPSILON) {
                if (localOrigin.x < -halfSize.x || localOrigin.x > halfSize.x) {
                    return false;
                }
            } else {
                float invDx = 1.0f / localDir.x;
                float t1    = (-halfSize.x - localOrigin.x) * invDx;
                float t2    = (halfSize.x - localOrigin.x) * invDx;
                Math::Vector2f nNear(-1, 0);
                if (t1 > t2) {
                    std::swap(t1, t2);
                    nNear = Math::Vector2f(1, 0);
                }
                if (t1 > tMin) {
                    tMin        = t1;
                    localNormal = nNear;
                }
                tMax = Math::Min(tMax, t2);
                if (tMin > tMax) {
                    return false;
                }
            }

            // Y slab
            if (Math::Abs(localDir.y) < Math::EPSILON) {
                if (localOrigin.y < -halfSize.y || localOrigin.y > halfSize.y) {
                    return false;
                }
            } else {
                float invDy = 1.0f / localDir.y;
                float t1    = (-halfSize.y - localOrigin.y) * invDy;
                float t2    = (halfSize.y - localOrigin.y) * invDy;
                Math::Vector2f nNear(0, -1);
                if (t1 > t2) {
                    std::swap(t1, t2);
                    nNear = Math::Vector2f(0, 1);
                }
                if (t1 > tMin) {
                    tMin        = t1;
                    localNormal = nNear;
                }
                tMax = Math::Min(tMax, t2);
                if (tMin > tMax) {
                    return false;
                }
            }

            if (tMin < 0.0f) {
                return false;
            }

            _outDistance = tMin;

            // Rotate normal back to world space
            float cosR = Math::Cos(_body.Angle);
            float sinR = Math::Sin(_body.Angle);
            _outNormal = Math::Vector2f(
                localNormal.x * cosR - localNormal.y * sinR, localNormal.x * sinR + localNormal.y * cosR);
            return true;
        }

        return false;
    }

    BodyHandle PhysicsService::PointQuery(Math::Vector2f _point) const {
        // Create a tiny AABB at the point for broadphase query
        Math::Bounds2D pointBounds(_point, Math::Vector2f(0.01f, 0.01f));

        BodyHandle result = BodyHandle::Invalid();
        mBroadphaseTree.Query(pointBounds, [&](int32 _proxyId) {
            if (result.IsValid()) {
                return; // Already found one
            }

            uint32 bodyIndex = mBroadphaseTree.GetBodyIndex(_proxyId);
            if (bodyIndex >= mBodies.size() || !mBodies[bodyIndex].bIsActive) {
                return;
            }

            const PhysicsBodyData& body = mBodies[bodyIndex];
            if (PointInBody(_point, body)) {
                result.Index      = bodyIndex;
                result.Generation = body.Generation;
            }
        });

        return result;
    }

    bool PhysicsService::Raycast(
        Math::Vector2f _origin, Math::Vector2f _direction, float _maxDistance, RaycastHit& _hit) const {
        // Normalize direction
        float dirLen = _direction.Magnitude();
        if (dirLen < Math::EPSILON) {
            return false;
        }
        _direction = _direction * (1.0f / dirLen);

        Math::Vector2f invDir(Math::Abs(_direction.x) > Math::EPSILON ? 1.0f / _direction.x : 1e18f,
            Math::Abs(_direction.y) > Math::EPSILON ? 1.0f / _direction.y : 1e18f);

        float closestDist = _maxDistance;
        bool bHit         = false;

        mBroadphaseTree.RayCast(_origin, invDir, _maxDistance, [&](int32 _proxyId) {
            uint32 bodyIndex = mBroadphaseTree.GetBodyIndex(_proxyId);
            if (bodyIndex >= mBodies.size() || !mBodies[bodyIndex].bIsActive) {
                return;
            }

            const PhysicsBodyData& body = mBodies[bodyIndex];
            float dist                  = 0.0f;
            Math::Vector2f normal;

            if (RaycastBody(_origin, _direction, closestDist, body, dist, normal)) {
                if (dist < closestDist) {
                    closestDist            = dist;
                    _hit.Handle.Index      = bodyIndex;
                    _hit.Handle.Generation = body.Generation;
                    _hit.Point             = _origin + _direction * dist;
                    _hit.Normal            = normal;
                    _hit.Distance          = dist;
                    bHit                   = true;
                }
            }
        });

        return bHit;
    }

    Vector<RaycastHit> PhysicsService::RaycastAll(
        Math::Vector2f _origin, Math::Vector2f _direction, float _maxDistance) const {
        // Normalize direction
        float dirLen = _direction.Magnitude();
        if (dirLen < Math::EPSILON) {
            return {};
        }
        _direction = _direction * (1.0f / dirLen);

        Math::Vector2f invDir(Math::Abs(_direction.x) > Math::EPSILON ? 1.0f / _direction.x : 1e18f,
            Math::Abs(_direction.y) > Math::EPSILON ? 1.0f / _direction.y : 1e18f);

        Vector<RaycastHit> hits;

        mBroadphaseTree.RayCast(_origin, invDir, _maxDistance, [&](int32 _proxyId) {
            uint32 bodyIndex = mBroadphaseTree.GetBodyIndex(_proxyId);
            if (bodyIndex >= mBodies.size() || !mBodies[bodyIndex].bIsActive) {
                return;
            }

            const PhysicsBodyData& body = mBodies[bodyIndex];
            float dist                  = 0.0f;
            Math::Vector2f normal;

            if (RaycastBody(_origin, _direction, _maxDistance, body, dist, normal)) {
                RaycastHit hit;
                hit.Handle.Index      = bodyIndex;
                hit.Handle.Generation = body.Generation;
                hit.Point             = _origin + _direction * dist;
                hit.Normal            = normal;
                hit.Distance          = dist;
                hits.emplace_back(hit);
            }
        });

        // Sort by distance (nearest first)
        std::sort(hits.begin(), hits.end(),
            [](const RaycastHit& _a, const RaycastHit& _b) { return _a.Distance < _b.Distance; });

        return hits;
    }

    // ============== Constraint Management ==============

    ConstraintHandle PhysicsService::CreateSpring(const SpringDef& _def) {
        if (!IsBodyValid(_def.BodyA)) {
            return ConstraintHandle::Invalid();
        }

        uint32 index;
        uint32 generation;
        if (!mFreeSpringIndices.empty()) {
            index = mFreeSpringIndices.back();
            mFreeSpringIndices.pop_back();
            generation = mSprings[index].Generation + 1;
        } else {
            index      = static_cast<uint32>(mSprings.size());
            generation = 0;
            mSprings.emplace_back();
        }

        SpringConstraintData& spring = mSprings[index];
        spring.HandleA               = _def.BodyA;
        spring.HandleB               = _def.BodyB;
        spring.LocalAnchorA          = _def.LocalAnchorA;
        spring.LocalAnchorB          = _def.LocalAnchorB;
        spring.WorldAnchorB          = _def.WorldAnchorB;
        spring.Stiffness             = _def.Stiffness;
        spring.Damping               = _def.Damping;
        spring.RestLength            = _def.RestLength;
        spring.bIsActive             = true;
        spring.Generation            = generation;

        // Auto-calculate rest length from initial positions if zero
        if (spring.RestLength <= 0.0f) {
            const PhysicsBodyData& bodyA = mBodies[_def.BodyA.Index];
            float cA                     = Math::Cos(bodyA.Angle);
            float sA                     = Math::Sin(bodyA.Angle);
            Math::Vector2f worldA        = bodyA.Position
                                  + Math::Vector2f(_def.LocalAnchorA.x * cA - _def.LocalAnchorA.y * sA,
                                      _def.LocalAnchorA.x * sA + _def.LocalAnchorA.y * cA);

            Math::Vector2f worldB;
            if (IsBodyValid(_def.BodyB)) {
                const PhysicsBodyData& bodyB = mBodies[_def.BodyB.Index];
                float cB                     = Math::Cos(bodyB.Angle);
                float sB                     = Math::Sin(bodyB.Angle);
                worldB                       = bodyB.Position
                       + Math::Vector2f(_def.LocalAnchorB.x * cB - _def.LocalAnchorB.y * sB,
                           _def.LocalAnchorB.x * sB + _def.LocalAnchorB.y * cB);
            } else {
                worldB = _def.WorldAnchorB;
            }
            spring.RestLength = (worldB - worldA).Magnitude();
        }

        // Wake connected bodies
        WakeConstraintBodies(_def.BodyA, _def.BodyB);

        ConstraintHandle handle;
        handle.Index      = index;
        handle.Generation = generation;
        return handle;
    }

    ConstraintHandle PhysicsService::CreateDistanceConstraint(const DistanceDef& _def) {
        if (!IsBodyValid(_def.BodyA) || !IsBodyValid(_def.BodyB)) {
            return ConstraintHandle::Invalid();
        }

        uint32 index;
        uint32 generation;
        if (!mFreeDistanceIndices.empty()) {
            index = mFreeDistanceIndices.back();
            mFreeDistanceIndices.pop_back();
            generation = mDistanceConstraints[index].Generation + 1;
        } else {
            index      = static_cast<uint32>(mDistanceConstraints.size());
            generation = 0;
            mDistanceConstraints.emplace_back();
        }

        DistanceConstraintData& dc = mDistanceConstraints[index];
        dc.HandleA                 = _def.BodyA;
        dc.HandleB                 = _def.BodyB;
        dc.LocalAnchorA            = _def.LocalAnchorA;
        dc.LocalAnchorB            = _def.LocalAnchorB;
        dc.Distance                = _def.Distance;
        dc.ImpulseAccum            = 0.0f;
        dc.bIsActive               = true;
        dc.Generation              = generation;

        // Auto-calculate distance from initial positions if zero
        if (dc.Distance <= 0.0f) {
            const PhysicsBodyData& bodyA = mBodies[_def.BodyA.Index];
            const PhysicsBodyData& bodyB = mBodies[_def.BodyB.Index];
            float cA                     = Math::Cos(bodyA.Angle);
            float sA                     = Math::Sin(bodyA.Angle);
            Math::Vector2f worldA        = bodyA.Position
                                  + Math::Vector2f(_def.LocalAnchorA.x * cA - _def.LocalAnchorA.y * sA,
                                      _def.LocalAnchorA.x * sA + _def.LocalAnchorA.y * cA);
            float cB              = Math::Cos(bodyB.Angle);
            float sB              = Math::Sin(bodyB.Angle);
            Math::Vector2f worldB = bodyB.Position
                                  + Math::Vector2f(_def.LocalAnchorB.x * cB - _def.LocalAnchorB.y * sB,
                                      _def.LocalAnchorB.x * sB + _def.LocalAnchorB.y * cB);
            dc.Distance = (worldB - worldA).Magnitude();
        }

        WakeConstraintBodies(_def.BodyA, _def.BodyB);

        ConstraintHandle handle;
        handle.Index      = index;
        handle.Generation = generation;
        return handle;
    }

    ConstraintHandle PhysicsService::CreateHinge(const HingeDef& _def) {
        if (!IsBodyValid(_def.BodyA) || !IsBodyValid(_def.BodyB)) {
            return ConstraintHandle::Invalid();
        }

        uint32 index;
        uint32 generation;
        if (!mFreeHingeIndices.empty()) {
            index = mFreeHingeIndices.back();
            mFreeHingeIndices.pop_back();
            generation = mHingeConstraints[index].Generation + 1;
        } else {
            index      = static_cast<uint32>(mHingeConstraints.size());
            generation = 0;
            mHingeConstraints.emplace_back();
        }

        HingeConstraintData& hc = mHingeConstraints[index];
        hc.HandleA              = _def.BodyA;
        hc.HandleB              = _def.BodyB;
        hc.LocalAnchorA         = _def.LocalAnchorA;
        hc.LocalAnchorB         = _def.LocalAnchorB;
        hc.bEnableLimits        = _def.bEnableLimits;
        hc.LowerAngle           = _def.LowerAngle;
        hc.UpperAngle           = _def.UpperAngle;
        hc.bEnableMotor         = _def.bEnableMotor;
        hc.MotorSpeed           = _def.MotorSpeed;
        hc.MaxMotorTorque       = _def.MaxMotorTorque;
        hc.ImpulseAccum         = Math::Vector2f(0, 0);
        hc.AngleImpulseAccum    = 0.0f;
        hc.MotorImpulseAccum    = 0.0f;
        hc.bIsActive            = true;
        hc.Generation           = generation;

        WakeConstraintBodies(_def.BodyA, _def.BodyB);

        ConstraintHandle handle;
        handle.Index      = index;
        handle.Generation = generation;
        return handle;
    }

    void PhysicsService::DestroyConstraint(ConstraintHandle _handle) {
        if (!_handle.IsValid()) {
            return;
        }

        // Check springs
        if (_handle.Index < mSprings.size() && mSprings[_handle.Index].bIsActive
            && mSprings[_handle.Index].Generation == _handle.Generation) {
            mSprings[_handle.Index].bIsActive = false;
            mFreeSpringIndices.push_back(_handle.Index);
            return;
        }

        // Check distance constraints
        if (_handle.Index < mDistanceConstraints.size() && mDistanceConstraints[_handle.Index].bIsActive
            && mDistanceConstraints[_handle.Index].Generation == _handle.Generation) {
            mDistanceConstraints[_handle.Index].bIsActive = false;
            mFreeDistanceIndices.push_back(_handle.Index);
            return;
        }

        // Check hinge constraints
        if (_handle.Index < mHingeConstraints.size() && mHingeConstraints[_handle.Index].bIsActive
            && mHingeConstraints[_handle.Index].Generation == _handle.Generation) {
            mHingeConstraints[_handle.Index].bIsActive = false;
            mFreeHingeIndices.push_back(_handle.Index);
            return;
        }
    }

    bool PhysicsService::IsConstraintValid(ConstraintHandle _handle) const {
        if (!_handle.IsValid()) {
            return false;
        }

        // Check all constraint types
        if (_handle.Index < mSprings.size() && mSprings[_handle.Index].bIsActive
            && mSprings[_handle.Index].Generation == _handle.Generation) {
            return true;
        }
        if (_handle.Index < mDistanceConstraints.size() && mDistanceConstraints[_handle.Index].bIsActive
            && mDistanceConstraints[_handle.Index].Generation == _handle.Generation) {
            return true;
        }
        if (_handle.Index < mHingeConstraints.size() && mHingeConstraints[_handle.Index].bIsActive
            && mHingeConstraints[_handle.Index].Generation == _handle.Generation) {
            return true;
        }
        return false;
    }

    // ============== Constraint Getters/Setters ==============

    void PhysicsService::SetSpringStiffness(ConstraintHandle _handle, float _stiffness) {
        if (_handle.IsValid() && _handle.Index < mSprings.size() && mSprings[_handle.Index].bIsActive
            && mSprings[_handle.Index].Generation == _handle.Generation) {
            mSprings[_handle.Index].Stiffness = _stiffness;
        }
    }

    void PhysicsService::SetSpringDamping(ConstraintHandle _handle, float _damping) {
        if (_handle.IsValid() && _handle.Index < mSprings.size() && mSprings[_handle.Index].bIsActive
            && mSprings[_handle.Index].Generation == _handle.Generation) {
            mSprings[_handle.Index].Damping = _damping;
        }
    }

    void PhysicsService::SetSpringRestLength(ConstraintHandle _handle, float _restLength) {
        if (_handle.IsValid() && _handle.Index < mSprings.size() && mSprings[_handle.Index].bIsActive
            && mSprings[_handle.Index].Generation == _handle.Generation) {
            mSprings[_handle.Index].RestLength = _restLength;
        }
    }

    void PhysicsService::SetHingeMotorEnabled(ConstraintHandle _handle, bool _bEnable) {
        if (_handle.IsValid() && _handle.Index < mHingeConstraints.size() && mHingeConstraints[_handle.Index].bIsActive
            && mHingeConstraints[_handle.Index].Generation == _handle.Generation) {
            mHingeConstraints[_handle.Index].bEnableMotor = _bEnable;
        }
    }

    void PhysicsService::SetHingeMotorSpeed(ConstraintHandle _handle, float _speed) {
        if (_handle.IsValid() && _handle.Index < mHingeConstraints.size() && mHingeConstraints[_handle.Index].bIsActive
            && mHingeConstraints[_handle.Index].Generation == _handle.Generation) {
            mHingeConstraints[_handle.Index].MotorSpeed = _speed;
        }
    }

    void PhysicsService::SetHingeMaxMotorTorque(ConstraintHandle _handle, float _maxTorque) {
        if (_handle.IsValid() && _handle.Index < mHingeConstraints.size() && mHingeConstraints[_handle.Index].bIsActive
            && mHingeConstraints[_handle.Index].Generation == _handle.Generation) {
            mHingeConstraints[_handle.Index].MaxMotorTorque = _maxTorque;
        }
    }

    void PhysicsService::SetHingeLimitsEnabled(ConstraintHandle _handle, bool _bEnable) {
        if (_handle.IsValid() && _handle.Index < mHingeConstraints.size() && mHingeConstraints[_handle.Index].bIsActive
            && mHingeConstraints[_handle.Index].Generation == _handle.Generation) {
            mHingeConstraints[_handle.Index].bEnableLimits = _bEnable;
        }
    }

    void PhysicsService::SetHingeLimits(ConstraintHandle _handle, float _lower, float _upper) {
        if (_handle.IsValid() && _handle.Index < mHingeConstraints.size() && mHingeConstraints[_handle.Index].bIsActive
            && mHingeConstraints[_handle.Index].Generation == _handle.Generation) {
            mHingeConstraints[_handle.Index].LowerAngle = _lower;
            mHingeConstraints[_handle.Index].UpperAngle = _upper;
        }
    }

    // ============== Constraint Solver Implementation ==============

    void PhysicsService::ApplySpringForces(float _deltaTime) {
        for (auto& spring : mSprings) {
            if (!spring.bIsActive) {
                continue;
            }
            if (!IsBodyValid(spring.HandleA)) {
                continue;
            }

            PhysicsBodyData& bodyA = mBodies[spring.HandleA.Index];
            if (bodyA.IsStatic() && bodyA.bIsKinematic) {
                continue;
            }

            // Compute world-space anchor positions
            float cosA                  = Math::Cos(bodyA.Angle);
            float sinA                  = Math::Sin(bodyA.Angle);
            Math::Vector2f worldAnchorA = bodyA.Position
                                        + Math::Vector2f(spring.LocalAnchorA.x * cosA - spring.LocalAnchorA.y * sinA,
                                            spring.LocalAnchorA.x * sinA + spring.LocalAnchorA.y * cosA);

            Math::Vector2f worldAnchorB;
            PhysicsBodyData* pBodyB = nullptr;
            if (IsBodyValid(spring.HandleB)) {
                pBodyB       = &mBodies[spring.HandleB.Index];
                float cosB   = Math::Cos(pBodyB->Angle);
                float sinB   = Math::Sin(pBodyB->Angle);
                worldAnchorB = pBodyB->Position
                             + Math::Vector2f(spring.LocalAnchorB.x * cosB - spring.LocalAnchorB.y * sinB,
                                 spring.LocalAnchorB.x * sinB + spring.LocalAnchorB.y * cosB);
            } else {
                worldAnchorB = spring.WorldAnchorB;
            }

            Math::Vector2f delta = worldAnchorB - worldAnchorA;
            float currentLength  = delta.Magnitude();
            if (currentLength < Math::EPSILON) {
                continue;
            }

            Math::Vector2f direction = delta * (1.0f / currentLength);

            // Relative velocity along spring axis
            Math::Vector2f velA = bodyA.Velocity;
            Math::Vector2f velB = pBodyB ? pBodyB->Velocity : Math::Vector2f(0, 0);
            float relVel        = Math::Vector2f::Dot(velB - velA, direction);

            // Hooke's law + damping: F = k * (x - x0) + c * v_rel
            float forceMag       = spring.Stiffness * (currentLength - spring.RestLength) + spring.Damping * relVel;
            Math::Vector2f force = direction * forceMag;

            // Apply force to body A (pulled toward B)
            if (!bodyA.IsStatic() && !bodyA.bIsKinematic) {
                if (Math::Abs(forceMag) > Math::EPSILON) {
                    bodyA.Wake();
                }
                bodyA.ForceAccumulated += force;
                Math::Vector2f rA = worldAnchorA - bodyA.Position;
                bodyA.TorqueAccumulated += Math::Vector2f::Cross2D(rA, force);
            }

            // Apply opposite force to body B
            if (pBodyB && !pBodyB->IsStatic() && !pBodyB->bIsKinematic) {
                if (Math::Abs(forceMag) > Math::EPSILON) {
                    pBodyB->Wake();
                }
                pBodyB->ForceAccumulated -= force;
                Math::Vector2f rB = worldAnchorB - pBodyB->Position;
                pBodyB->TorqueAccumulated -= Math::Vector2f::Cross2D(rB, force);
            }
        }
    }

    void PhysicsService::PrecomputeConstraints(float _deltaTime) {
        float invDt           = _deltaTime > 0.0f ? 1.0f / _deltaTime : 0.0f;
        const float baumgarte = 0.2f;

        // Precompute distance constraints
        for (auto& dc : mDistanceConstraints) {
            if (!dc.bIsActive || !IsBodyValid(dc.HandleA) || !IsBodyValid(dc.HandleB)) {
                continue;
            }

            PhysicsBodyData& bodyA = mBodies[dc.HandleA.Index];
            PhysicsBodyData& bodyB = mBodies[dc.HandleB.Index];

            float invMassA    = bodyA.bIsSleeping ? 0.0f : bodyA.InverseMass;
            float invInertiaA = bodyA.bIsSleeping ? 0.0f : bodyA.InverseInertia;
            float invMassB    = bodyB.bIsSleeping ? 0.0f : bodyB.InverseMass;
            float invInertiaB = bodyB.bIsSleeping ? 0.0f : bodyB.InverseInertia;

            // Compute lever arms in world space
            float cosA = Math::Cos(bodyA.Angle);
            float sinA = Math::Sin(bodyA.Angle);
            dc.rA      = Math::Vector2f(dc.LocalAnchorA.x * cosA - dc.LocalAnchorA.y * sinA,
                     dc.LocalAnchorA.x * sinA + dc.LocalAnchorA.y * cosA);

            float cosB = Math::Cos(bodyB.Angle);
            float sinB = Math::Sin(bodyB.Angle);
            dc.rB      = Math::Vector2f(dc.LocalAnchorB.x * cosB - dc.LocalAnchorB.y * sinB,
                     dc.LocalAnchorB.x * sinB + dc.LocalAnchorB.y * cosB);

            Math::Vector2f worldA = bodyA.Position + dc.rA;
            Math::Vector2f worldB = bodyB.Position + dc.rB;
            Math::Vector2f delta  = worldB - worldA;
            float currentDist     = delta.Magnitude();

            if (currentDist > Math::EPSILON) {
                dc.Axis = delta * (1.0f / currentDist);
            } else {
                dc.Axis = Math::Vector2f(1, 0);
            }

            // Effective mass along constraint axis
            float rACrossN = Math::Vector2f::Cross2D(dc.rA, dc.Axis);
            float rBCrossN = Math::Vector2f::Cross2D(dc.rB, dc.Axis);
            float denom = invMassA + invMassB + rACrossN * rACrossN * invInertiaA + rBCrossN * rBCrossN * invInertiaB;
            dc.EffectiveMass = denom > 0.0f ? 1.0f / denom : 0.0f;

            // Baumgarte position correction bias
            dc.Bias = baumgarte * invDt * (currentDist - dc.Distance);

            // Reset accumulated impulse each frame (no warm-starting for simplicity)
            dc.ImpulseAccum = 0.0f;
        }

        // Precompute hinge constraints
        for (auto& hc : mHingeConstraints) {
            if (!hc.bIsActive || !IsBodyValid(hc.HandleA) || !IsBodyValid(hc.HandleB)) {
                continue;
            }

            PhysicsBodyData& bodyA = mBodies[hc.HandleA.Index];
            PhysicsBodyData& bodyB = mBodies[hc.HandleB.Index];

            float invMassA    = bodyA.bIsSleeping ? 0.0f : bodyA.InverseMass;
            float invInertiaA = bodyA.bIsSleeping ? 0.0f : bodyA.InverseInertia;
            float invMassB    = bodyB.bIsSleeping ? 0.0f : bodyB.InverseMass;
            float invInertiaB = bodyB.bIsSleeping ? 0.0f : bodyB.InverseInertia;

            // Compute lever arms
            float cosA = Math::Cos(bodyA.Angle);
            float sinA = Math::Sin(bodyA.Angle);
            hc.rA      = Math::Vector2f(hc.LocalAnchorA.x * cosA - hc.LocalAnchorA.y * sinA,
                     hc.LocalAnchorA.x * sinA + hc.LocalAnchorA.y * cosA);

            float cosB = Math::Cos(bodyB.Angle);
            float sinB = Math::Sin(bodyB.Angle);
            hc.rB      = Math::Vector2f(hc.LocalAnchorB.x * cosB - hc.LocalAnchorB.y * sinB,
                     hc.LocalAnchorB.x * sinB + hc.LocalAnchorB.y * cosB);

            // 2x2 effective mass matrix K for point constraint
            // K = [invMA + invMB + rAy^2*invIA + rBy^2*invIB,   -rAx*rAy*invIA - rBx*rBy*invIB]
            //     [-rAx*rAy*invIA - rBx*rBy*invIB,               invMA + invMB + rAx^2*invIA + rBx^2*invIB]
            float totalInvMass = invMassA + invMassB;
            hc.Kxx             = totalInvMass + hc.rA.y * hc.rA.y * invInertiaA + hc.rB.y * hc.rB.y * invInertiaB;
            hc.Kxy             = -hc.rA.x * hc.rA.y * invInertiaA - hc.rB.x * hc.rB.y * invInertiaB;
            hc.Kyx             = hc.Kxy;
            hc.Kyy             = totalInvMass + hc.rA.x * hc.rA.x * invInertiaA + hc.rB.x * hc.rB.x * invInertiaB;

            // Angular effective mass (for limits and motor)
            float angularInvMass  = invInertiaA + invInertiaB;
            hc.AngleEffectiveMass = angularInvMass > 0.0f ? 1.0f / angularInvMass : 0.0f;

            // Position error bias
            Math::Vector2f posError = (bodyB.Position + hc.rB) - (bodyA.Position + hc.rA);
            hc.PositionBias         = posError * (baumgarte * invDt);

            // Reset accumulators
            hc.ImpulseAccum      = Math::Vector2f(0, 0);
            hc.AngleImpulseAccum = 0.0f;
            hc.MotorImpulseAccum = 0.0f;
        }
    }

    void PhysicsService::SolveConstraintVelocities() {
        // Solve distance constraints
        for (auto& dc : mDistanceConstraints) {
            if (!dc.bIsActive || !IsBodyValid(dc.HandleA) || !IsBodyValid(dc.HandleB)) {
                continue;
            }

            PhysicsBodyData& bodyA = mBodies[dc.HandleA.Index];
            PhysicsBodyData& bodyB = mBodies[dc.HandleB.Index];

            float invMassA    = bodyA.bIsSleeping ? 0.0f : bodyA.InverseMass;
            float invInertiaA = bodyA.bIsSleeping ? 0.0f : bodyA.InverseInertia;
            float invMassB    = bodyB.bIsSleeping ? 0.0f : bodyB.InverseMass;
            float invInertiaB = bodyB.bIsSleeping ? 0.0f : bodyB.InverseInertia;

            // Compute relative velocity at anchor points along constraint axis
            Math::Vector2f velA = bodyA.Velocity + Math::Vector2f(-dc.rA.y, dc.rA.x) * bodyA.AngularVelocity;
            Math::Vector2f velB = bodyB.Velocity + Math::Vector2f(-dc.rB.y, dc.rB.x) * bodyB.AngularVelocity;
            float relVel        = Math::Vector2f::Dot(velB - velA, dc.Axis);

            float lambda = dc.EffectiveMass * -(relVel + dc.Bias);
            dc.ImpulseAccum += lambda;

            Math::Vector2f impulse = dc.Axis * lambda;
            bodyA.Velocity -= impulse * invMassA;
            bodyA.AngularVelocity -= Math::Vector2f::Cross2D(dc.rA, impulse) * invInertiaA;
            bodyB.Velocity += impulse * invMassB;
            bodyB.AngularVelocity += Math::Vector2f::Cross2D(dc.rB, impulse) * invInertiaB;
        }

        // Solve hinge constraints
        for (auto& hc : mHingeConstraints) {
            if (!hc.bIsActive || !IsBodyValid(hc.HandleA) || !IsBodyValid(hc.HandleB)) {
                continue;
            }

            PhysicsBodyData& bodyA = mBodies[hc.HandleA.Index];
            PhysicsBodyData& bodyB = mBodies[hc.HandleB.Index];

            float invMassA    = bodyA.bIsSleeping ? 0.0f : bodyA.InverseMass;
            float invInertiaA = bodyA.bIsSleeping ? 0.0f : bodyA.InverseInertia;
            float invMassB    = bodyB.bIsSleeping ? 0.0f : bodyB.InverseMass;
            float invInertiaB = bodyB.bIsSleeping ? 0.0f : bodyB.InverseInertia;

            // === Point constraint (2 DOF) ===
            Math::Vector2f velA = bodyA.Velocity + Math::Vector2f(-hc.rA.y, hc.rA.x) * bodyA.AngularVelocity;
            Math::Vector2f velB = bodyB.Velocity + Math::Vector2f(-hc.rB.y, hc.rB.x) * bodyB.AngularVelocity;
            Math::Vector2f Cdot = velB - velA;

            // Add position bias
            Math::Vector2f rhs = Cdot + hc.PositionBias;

            // Solve K * lambda = -rhs using Cramer's rule for 2x2
            float det = hc.Kxx * hc.Kyy - hc.Kxy * hc.Kyx;
            Math::Vector2f lambda;
            if (Math::Abs(det) > Math::EPSILON) {
                float invDet = 1.0f / det;
                lambda.x     = -(hc.Kyy * rhs.x - hc.Kxy * rhs.y) * invDet;
                lambda.y     = -(-hc.Kyx * rhs.x + hc.Kxx * rhs.y) * invDet;
            } else {
                lambda = Math::Vector2f(0, 0);
            }

            hc.ImpulseAccum += lambda;

            bodyA.Velocity -= lambda * invMassA;
            bodyA.AngularVelocity -= Math::Vector2f::Cross2D(hc.rA, lambda) * invInertiaA;
            bodyB.Velocity += lambda * invMassB;
            bodyB.AngularVelocity += Math::Vector2f::Cross2D(hc.rB, lambda) * invInertiaB;

            // === Motor ===
            if (hc.bEnableMotor) {
                float angVelError  = (bodyB.AngularVelocity - bodyA.AngularVelocity) - hc.MotorSpeed;
                float motorImpulse = hc.AngleEffectiveMass * -angVelError;

                float oldMotorAccum  = hc.MotorImpulseAccum;
                hc.MotorImpulseAccum = Math::Clamp(oldMotorAccum + motorImpulse, -hc.MaxMotorTorque, hc.MaxMotorTorque);
                motorImpulse         = hc.MotorImpulseAccum - oldMotorAccum;

                bodyA.AngularVelocity -= motorImpulse * invInertiaA;
                bodyB.AngularVelocity += motorImpulse * invInertiaB;
            }

            // === Angular limits ===
            if (hc.bEnableLimits) {
                float relAngle = bodyB.Angle - bodyA.Angle;

                // Normalize relative angle to [-PI, PI]
                while (relAngle > Math::PI) {
                    relAngle -= 2.0f * Math::PI;
                }
                while (relAngle < -Math::PI) {
                    relAngle += 2.0f * Math::PI;
                }

                float angularError = 0.0f;
                if (relAngle < hc.LowerAngle) {
                    angularError = relAngle - hc.LowerAngle;
                } else if (relAngle > hc.UpperAngle) {
                    angularError = relAngle - hc.UpperAngle;
                }

                if (Math::Abs(angularError) > Math::EPSILON) {
                    float relAngVel    = bodyB.AngularVelocity - bodyA.AngularVelocity;
                    float limitImpulse = hc.AngleEffectiveMass * -(relAngVel + angularError * 10.0f);

                    // Clamp: at lower limit, impulse must be >= 0; at upper, must be <= 0
                    float oldAccum = hc.AngleImpulseAccum;
                    if (relAngle < hc.LowerAngle) {
                        hc.AngleImpulseAccum = Math::Max(oldAccum + limitImpulse, 0.0f);
                    } else {
                        hc.AngleImpulseAccum = Math::Min(oldAccum + limitImpulse, 0.0f);
                    }
                    limitImpulse = hc.AngleImpulseAccum - oldAccum;

                    bodyA.AngularVelocity -= limitImpulse * invInertiaA;
                    bodyB.AngularVelocity += limitImpulse * invInertiaB;
                }
            }
        }
    }

    void PhysicsService::SolveConstraintPositions() {
        const float baumgarte = 0.2f;
        const float slop      = 0.005f;

        // Position correction for distance constraints
        for (auto& dc : mDistanceConstraints) {
            if (!dc.bIsActive || !IsBodyValid(dc.HandleA) || !IsBodyValid(dc.HandleB)) {
                continue;
            }

            PhysicsBodyData& bodyA = mBodies[dc.HandleA.Index];
            PhysicsBodyData& bodyB = mBodies[dc.HandleB.Index];

            float invMassA = bodyA.bIsSleeping ? 0.0f : bodyA.InverseMass;
            float invMassB = bodyB.bIsSleeping ? 0.0f : bodyB.InverseMass;

            // Recompute world anchors from current positions
            float cosA = Math::Cos(bodyA.Angle);
            float sinA = Math::Sin(bodyA.Angle);
            Math::Vector2f rA(dc.LocalAnchorA.x * cosA - dc.LocalAnchorA.y * sinA,
                dc.LocalAnchorA.x * sinA + dc.LocalAnchorA.y * cosA);

            float cosB = Math::Cos(bodyB.Angle);
            float sinB = Math::Sin(bodyB.Angle);
            Math::Vector2f rB(dc.LocalAnchorB.x * cosB - dc.LocalAnchorB.y * sinB,
                dc.LocalAnchorB.x * sinB + dc.LocalAnchorB.y * cosB);

            Math::Vector2f worldA = bodyA.Position + rA;
            Math::Vector2f worldB = bodyB.Position + rB;
            Math::Vector2f delta  = worldB - worldA;
            float currentDist     = delta.Magnitude();

            float error = currentDist - dc.Distance;
            if (Math::Abs(error) <= slop) {
                continue;
            }

            Math::Vector2f n = currentDist > Math::EPSILON ? delta * (1.0f / currentDist) : Math::Vector2f(1, 0);
            float invMassSum = invMassA + invMassB;
            if (invMassSum <= 0.0f) {
                continue;
            }

            float correction = baumgarte * error / invMassSum;
            bodyA.Position += n * (correction * invMassA);
            bodyB.Position -= n * (correction * invMassB);
        }

        // Position correction for hinge constraints
        for (auto& hc : mHingeConstraints) {
            if (!hc.bIsActive || !IsBodyValid(hc.HandleA) || !IsBodyValid(hc.HandleB)) {
                continue;
            }

            PhysicsBodyData& bodyA = mBodies[hc.HandleA.Index];
            PhysicsBodyData& bodyB = mBodies[hc.HandleB.Index];

            float invMassA = bodyA.bIsSleeping ? 0.0f : bodyA.InverseMass;
            float invMassB = bodyB.bIsSleeping ? 0.0f : bodyB.InverseMass;

            // Recompute lever arms
            float cosA = Math::Cos(bodyA.Angle);
            float sinA = Math::Sin(bodyA.Angle);
            Math::Vector2f rA(hc.LocalAnchorA.x * cosA - hc.LocalAnchorA.y * sinA,
                hc.LocalAnchorA.x * sinA + hc.LocalAnchorA.y * cosA);

            float cosB = Math::Cos(bodyB.Angle);
            float sinB = Math::Sin(bodyB.Angle);
            Math::Vector2f rB(hc.LocalAnchorB.x * cosB - hc.LocalAnchorB.y * sinB,
                hc.LocalAnchorB.x * sinB + hc.LocalAnchorB.y * cosB);

            Math::Vector2f posError = (bodyB.Position + rB) - (bodyA.Position + rA);
            float errorMag          = posError.Magnitude();
            if (errorMag <= slop) {
                continue;
            }

            float invMassSum = invMassA + invMassB;
            if (invMassSum <= 0.0f) {
                continue;
            }

            Math::Vector2f correction = posError * (baumgarte / invMassSum);
            bodyA.Position += correction * invMassA;
            bodyB.Position -= correction * invMassB;
        }
    }

    void PhysicsService::DestroyConstraintsForBody(uint32 _bodyIndex) {
        for (uint32 i = 0; i < mSprings.size(); ++i) {
            auto& s = mSprings[i];
            if (!s.bIsActive) {
                continue;
            }
            if (s.HandleA.Index == _bodyIndex || (s.HandleB.IsValid() && s.HandleB.Index == _bodyIndex)) {
                s.bIsActive = false;
                mFreeSpringIndices.push_back(i);
            }
        }

        for (uint32 i = 0; i < mDistanceConstraints.size(); ++i) {
            auto& dc = mDistanceConstraints[i];
            if (!dc.bIsActive) {
                continue;
            }
            if (dc.HandleA.Index == _bodyIndex || dc.HandleB.Index == _bodyIndex) {
                dc.bIsActive = false;
                mFreeDistanceIndices.push_back(i);
            }
        }

        for (uint32 i = 0; i < mHingeConstraints.size(); ++i) {
            auto& hc = mHingeConstraints[i];
            if (!hc.bIsActive) {
                continue;
            }
            if (hc.HandleA.Index == _bodyIndex || hc.HandleB.Index == _bodyIndex) {
                hc.bIsActive = false;
                mFreeHingeIndices.push_back(i);
            }
        }
    }

    void PhysicsService::WakeConstraintBodies(const BodyHandle& _handleA, const BodyHandle& _handleB) {
        if (IsBodyValid(_handleA)) {
            PhysicsBodyData& bodyA = mBodies[_handleA.Index];
            if (!bodyA.IsStatic()) {
                bodyA.Wake();
            }
        }
        if (IsBodyValid(_handleB)) {
            PhysicsBodyData& bodyB = mBodies[_handleB.Index];
            if (!bodyB.IsStatic()) {
                bodyB.Wake();
            }
        }
    }

} // namespace Umbra
