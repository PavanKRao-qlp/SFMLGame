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
        // 1. Integrate forces -> accelerations -> velocities
        IntegrateForces(_deltaTime);

        // 2. Integrate velocities -> positions
        IntegrateVelocities(_deltaTime);

        // 3. Apply damping
        ApplyDamping(_deltaTime);

        // 4. Clear force accumulators for next frame
        ClearForceAccumulators();

        // 5. Update broadphase tree proxies
        UpdateBroadphaseProxies(_deltaTime);

        // 6. Check broad phase collisions
        BroadphaseDetection();

        // 7. Check Narrow phase collision for contacts
        NarrowPhaseDetection();

        // 8. Precompute constraint data (effective masses, bias) once per frame
        PrecomputeContactConstraints();

        // 9. Velocity solver - sequential impulse with accumulated clamping
        for (int i = 0; i < mConfig.VelocityIterations; i++) {
            ResolveContacts();
        }

        // 10. Position correction iterations (separate from velocity)
        for (int i = 0; i < mConfig.PositionIterations; i++) {
            PositionContraction();
        }

        // 11. Sleep or Wake bodies
        UpdateSleepingBodies(_deltaTime);
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

            // Normalize angle to [0, 2*PI)
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
            collisionDef.handleA  = std::get<0>(handlePair);
            collisionDef.handleB  = std::get<1>(handlePair);
            PhysicsBodyData bodyA = mBodies[collisionDef.handleA.Index];
            PhysicsBodyData bodyB = mBodies[collisionDef.handleB.Index];
            if (CollisionQuery::CheckCollision(bodyA, bodyB, collisionDef)) {
                mCollisions.emplace_back(collisionDef);
            }
        }
    }

    void PhysicsService::PrecomputeContactConstraints() {
        for (CollisionDef& collision : mCollisions) {
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
                float rACrossN    = Math::Vector2f::Cross2D(contact.rA, collision.contactNormal);
                float rBCrossN    = Math::Vector2f::Cross2D(contact.rB, collision.contactNormal);
                float normalDenom = invMassSum + rACrossN * rACrossN * invInertiaA
                                  + rBCrossN * rBCrossN * invInertiaB;
                contact.normalMass = normalDenom > 0.0f ? 1.0f / normalDenom : 0.0f;

                // Effective mass along the tangent (same formula, tangent direction)
                float rACrossT     = Math::Vector2f::Cross2D(contact.rA, tangent);
                float rBCrossT     = Math::Vector2f::Cross2D(contact.rB, tangent);
                float tangentDenom = invMassSum + rACrossT * rACrossT * invInertiaA
                                   + rBCrossT * rBCrossT * invInertiaB;
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
        // A sleeping body should wake if it's colliding with an awake body
        for (const CollisionDef& collisionDef : mCollisions) {
            PhysicsBodyData& bodyA = *GetBodyData(collisionDef.handleA);
            PhysicsBodyData& bodyB = *GetBodyData(collisionDef.handleB);

            // Skip if both bodies are static
            if (bodyA.IsStatic() && bodyB.IsStatic()) {
                continue;
            }
            // If one is sleeping and the other is awake and moving, wake the sleeper
            bool bAIsAsleep = bodyA.bIsSleeping;
            bool bBIsAsleep = bodyB.bIsSleeping;
            if (bodyA.bIsSleeping && !bodyB.bIsSleeping && !bodyB.IsStatic()) {
                // B is awake and dynamic, wake A
                bodyA.Wake();
            }
            if (bodyB.bIsSleeping && !bodyA.bIsSleeping && !bodyA.IsStatic()) {
                // A is awake and dynamic, wake B
                bodyB.Wake();
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

} // namespace Umbra
