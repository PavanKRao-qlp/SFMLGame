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

    const Vector<CollisionEvent>& PhysicsService::GetCollisionEnterEvents() const {
        return mCollisionEnterEvents;
    }

    const Vector<CollisionEvent>& PhysicsService::GetCollisionStayEvents() const {
        return mCollisionStayEvents;
    }

    const Vector<CollisionEvent>& PhysicsService::GetCollisionExitEvents() const {
        return mCollisionExitEvents;
    }

    uint64 PhysicsService::MakeCollisionPairKey(uint32 _indexA, uint32 _indexB) {
        uint32 lo = _indexA < _indexB ? _indexA : _indexB;
        uint32 hi = _indexA < _indexB ? _indexB : _indexA;
        return (static_cast<uint64>(lo) << 32) | static_cast<uint64>(hi);
    }

    void PhysicsService::CategorizeCollisionEvents() {
        mCollisionEnterEvents.clear();
        mCollisionStayEvents.clear();
        mCollisionExitEvents.clear();

        Set<uint64> currentPairs;

        for (const CollisionDef& collision : mCollisions) {
            uint64 key = MakeCollisionPairKey(collision.handleA.Index, collision.handleB.Index);
            currentPairs.insert(key);

            CollisionEvent event;
            event.HandleA       = collision.handleA;
            event.HandleB       = collision.handleB;
            event.ContactNormal = collision.contactNormal;
            event.Penetration   = collision.penetration;

            if (mPreviousCollisionPairs.find(key) != mPreviousCollisionPairs.end()) {
                mCollisionStayEvents.emplace_back(event);
            } else {
                mCollisionEnterEvents.emplace_back(event);
            }
        }

        // Pairs that were colliding last frame but not this frame → Exit
        for (uint64 prevKey : mPreviousCollisionPairs) {
            if (currentPairs.find(prevKey) == currentPairs.end()) {
                CollisionEvent event;
                event.HandleA.Index      = static_cast<uint32>(prevKey >> 32);
                event.HandleB.Index      = static_cast<uint32>(prevKey & 0xFFFFFFFF);
                // Reconstruct generations from current body data if still valid
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

        mPreviousCollisionPairs = std::move(currentPairs);
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
            float dx = _point.x - _body.Position.x;
            float dy = _point.y - _body.Position.y;
            float cosA = Math::Cos(-_body.Angle);
            float sinA = Math::Sin(-_body.Angle);
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

            float a    = Math::Vector2f::Dot(_direction, _direction);
            Math::Vector2f d(dx, dy);
            float b    = 2.0f * Math::Vector2f::Dot(d, _direction);
            float c    = Math::Vector2f::Dot(d, d) - r * r;
            float disc = b * b - 4.0f * a * c;

            if (disc < 0.0f) {
                return false;
            }

            float sqrtDisc = Math::Sqrt(disc);
            float t = (-b - sqrtDisc) / (2.0f * a);

            // If the near root is behind us, try the far root (origin inside circle)
            if (t < 0.0f) {
                t = (-b + sqrtDisc) / (2.0f * a);
            }

            if (t < 0.0f || t > _maxDistance) {
                return false;
            }

            _outDistance = t;
            Math::Vector2f hitPoint = _origin + _direction * t;
            _outNormal = hitPoint - _body.Position;
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
            Math::Vector2f localDir(_direction.x * cosA - _direction.y * sinA,
                _direction.x * sinA + _direction.y * cosA);

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
                float t1 = (-halfSize.x - localOrigin.x) * invDx;
                float t2 = (halfSize.x - localOrigin.x) * invDx;
                Math::Vector2f nNear(-1, 0);
                if (t1 > t2) {
                    std::swap(t1, t2);
                    nNear = Math::Vector2f(1, 0);
                }
                if (t1 > tMin) {
                    tMin = t1;
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
                float t1 = (-halfSize.y - localOrigin.y) * invDy;
                float t2 = (halfSize.y - localOrigin.y) * invDy;
                Math::Vector2f nNear(0, -1);
                if (t1 > t2) {
                    std::swap(t1, t2);
                    nNear = Math::Vector2f(0, 1);
                }
                if (t1 > tMin) {
                    tMin = t1;
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
                localNormal.x * cosR - localNormal.y * sinR,
                localNormal.x * sinR + localNormal.y * cosR);
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

        Math::Vector2f invDir(
            Math::Abs(_direction.x) > Math::EPSILON ? 1.0f / _direction.x : 1e18f,
            Math::Abs(_direction.y) > Math::EPSILON ? 1.0f / _direction.y : 1e18f);

        float closestDist = _maxDistance;
        bool bHit         = false;

        mBroadphaseTree.RayCast(
            _origin, invDir, _maxDistance, [&](int32 _proxyId) {
                uint32 bodyIndex = mBroadphaseTree.GetBodyIndex(_proxyId);
                if (bodyIndex >= mBodies.size() || !mBodies[bodyIndex].bIsActive) {
                    return;
                }

                const PhysicsBodyData& body = mBodies[bodyIndex];
                float dist                  = 0.0f;
                Math::Vector2f normal;

                if (RaycastBody(_origin, _direction, closestDist, body, dist, normal)) {
                    if (dist < closestDist) {
                        closestDist         = dist;
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

        Math::Vector2f invDir(
            Math::Abs(_direction.x) > Math::EPSILON ? 1.0f / _direction.x : 1e18f,
            Math::Abs(_direction.y) > Math::EPSILON ? 1.0f / _direction.y : 1e18f);

        Vector<RaycastHit> hits;

        mBroadphaseTree.RayCast(
            _origin, invDir, _maxDistance, [&](int32 _proxyId) {
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

} // namespace Umbra
