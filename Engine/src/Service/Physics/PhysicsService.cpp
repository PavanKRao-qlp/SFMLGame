#include "Service/Physics/PhysicsService.h"

#include "Math/MathUtils.h"
#include "Service/Physics/CollisionQuery.h"

namespace Umbra {

    PhysicsService::PhysicsService() : PhysicsService(PhysicsServiceConfig{}) {}

    PhysicsService::PhysicsService(const PhysicsServiceConfig& _config) : mConfig(_config) {
        mBodies.reserve(_config.InitialBodyCapacity);
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

        // 5. Check broad phase collisions
        BroadphaseDetection();

        // 6. Check Narrow phase collision for contacts
        NarrowPhaseDetection();

        // 7. Resolve Contacts
        // Multiple iterations improve stability for stacking
        for (int i = 0; i < mConfig.VelocityIterations; i++) {
            ResolveContacts();
            PositionContraction();
        }
    }

    void PhysicsService::IntegrateForces(float _deltaTime) {
        for (auto& body : mBodies) {
            if (!body.bIsActive || body.IsStatic() || body.bIsKinematic) {
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
            if (!body.bIsActive || body.IsStatic() || body.bIsKinematic) {
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
            if (!body.bIsActive || body.IsStatic() || body.bIsKinematic) {
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


    void PhysicsService::BroadphaseDetection() {
        // clear last frame overlap
        mOverlappingBoundsIndexPair.clear();
        // Early out if less than 2 bodies
        if (mBodies.size() < 2) {
            return;
        }
        for (int i = 0; i < mBodies.size(); i++) {
            mBodies[i].UpdateBoundingAABB();
            for (int j = i + 1; j < mBodies.size(); j++) {
                mBodies[j].UpdateBoundingAABB();
                if (mBodies[i].BoundingAABB.Intersects(mBodies[j].BoundingAABB)) {
                    mOverlappingBoundsIndexPair.emplace_back(i, j);
                }
            }
        }
    }


    void PhysicsService::NarrowPhaseDetection() {
        // clear last frame Collision
        mCollisions.clear();
        for (auto indexPair : mOverlappingBoundsIndexPair) {
            CollisionDef collisionDef;
            collisionDef.indexA   = std::get<0>(indexPair);
            collisionDef.indexB   = std::get<1>(indexPair);
            PhysicsBodyData bodyA = mBodies[collisionDef.indexA];
            PhysicsBodyData bodyB = mBodies[collisionDef.indexB];
            if (CollisionQuery::CheckCollision(bodyA, bodyB, collisionDef)) {
                mCollisions.emplace_back(collisionDef);
            }
        }
    }

    void PhysicsService::ResolveContacts() {
        for (const CollisionDef& collisionDef : mCollisions) {
            PhysicsBodyData& bodyA = mBodies[collisionDef.indexA];
            PhysicsBodyData& bodyB = mBodies[collisionDef.indexB];

            if (bodyA.IsStatic() && bodyB.IsStatic()) {
                // static object , objects are unmovable
                continue;
            }
            // After collision due to Conservation of Momentum we know
            // massA * velA +  massB * velB = massA * newVelA + massB * newVelB;
            //

            // Calculate relative velocity at contact point
            // v_rel = v_B - v_A (including angular velocity contribution)
            Math::Vector2f rA = collisionDef.contacts[0].contactPoint - bodyA.Position;
            Math::Vector2f rB = collisionDef.contacts[0].contactPoint - bodyB.Position;

            // Velocity at contact point = linear velocity + angular velocity x r (since rotation contributes to
            // velocity change of point)
            // In 2D : v_contact = v + omega * perp(r) where perp(r) = (-r.y, r.x)
            Math::Vector2f velA = bodyA.Velocity + Math::Vector2f(-rA.y, rA.x) * bodyA.AngularVelocity;
            Math::Vector2f velB = bodyB.Velocity + Math::Vector2f(-rB.y, rB.x) * bodyB.AngularVelocity;

            Math::Vector2f relativeVel = velB - velA;

            // Relative velocity along the contact normal
            float velAlongNormal = Math::Vector2f::Dot(relativeVel, collisionDef.contactNormal);

            // Don't resolve if velocities are separating
            if (velAlongNormal > 0.0f) {
                continue;
            }

            // Calculate restitution (use minimum of the two bodies)
            float e = Math::Min(bodyA.CoefOfRestitution, bodyB.CoefOfRestitution);

            // Calculate impulse scalar using the formula:
            // j = -(1 + e) * v_rel . n
            //     -------------------------
            //     1/m_A + 1/m_B + (r_A x n)^2/I_A + (r_B x n)^2/I_B
            float rACrossN = Math::Vector2f::Cross2D(rA, collisionDef.contactNormal);
            float rBCrossN = Math::Vector2f::Cross2D(rB, collisionDef.contactNormal);

            float invMassSum = bodyA.InverseMass + bodyB.InverseMass;
            float invInertiaSum =
                rACrossN * rACrossN * bodyA.InverseInertia + rBCrossN * rBCrossN * bodyB.InverseInertia;

            float j = -(1.0f + e) * velAlongNormal;
            j /= invMassSum + invInertiaSum;

            // Apply impulse
            Math::Vector2f impulse = collisionDef.contactNormal * j;
            BodyHandle handleA     = GetBodyHandle(bodyA);
            BodyHandle handleB     = GetBodyHandle(bodyB);
            ApplyImpulseAtPoint(handleA, impulse * -1, collisionDef.contacts[0].contactPoint);
            ApplyImpulseAtPoint(handleB, impulse * 1, collisionDef.contacts[0].contactPoint);

            // === FRICTION IMPULSE (Coulomb friction model) ===

            // Recalculate relative velocity after normal impulse
            velA        = bodyA.Velocity + Math::Vector2f(-rA.y, rA.x) * bodyA.AngularVelocity;
            velB        = bodyB.Velocity + Math::Vector2f(-rB.y, rB.x) * bodyB.AngularVelocity;
            relativeVel = velB - velA;

            // Calculate tangent vector (perpendicular to normal)
            // Remove the normal component from relative velocity to get tangent direction
            Math::Vector2f tangent =
                relativeVel - collisionDef.contactNormal * Math::Vector2f::Dot(relativeVel, collisionDef.contactNormal);
            float tangentLength = tangent.Magnitude();

            // Skip friction if no tangential velocity
            if (tangentLength < 0.0001f) {
                continue;
            }

            // Normalize tangent
            tangent = tangent / tangentLength;

            // Calculate friction impulse magnitude
            // Same formula as normal impulse but along tangent direction
            float rACrossT = Math::Vector2f::Cross2D(rA, tangent);
            float rBCrossT = Math::Vector2f::Cross2D(rB, tangent);

            float invInertiaSumT =
                rACrossT * rACrossT * bodyA.InverseInertia + rBCrossT * rBCrossT * bodyB.InverseInertia;

            float jt = -Math::Vector2f::Dot(relativeVel, tangent);
            jt /= invMassSum + invInertiaSumT;

            // Coulomb friction: clamp friction impulse by normal impulse * friction coefficient
            // Use geometric mean of friction coefficients
            float staticFriction  = Math::Sqrt(bodyA.StaticFriction * bodyB.StaticFriction);
            float dynamicFriction = Math::Sqrt(bodyA.DynamicFriction * bodyB.DynamicFriction);

            Math::Vector2f frictionImpulse;
            if (Math::Abs(jt) < j * staticFriction) {
                // Static friction - object not sliding yet
                frictionImpulse = tangent * jt;
            } else {
                // Dynamic friction - object is sliding
                frictionImpulse = tangent * (-j * dynamicFriction);
            }

            // Apply friction impulse
            ApplyImpulseAtPoint(handleA, frictionImpulse * -1.0f, collisionDef.contacts[0].contactPoint);
            ApplyImpulseAtPoint(handleB, frictionImpulse, collisionDef.contacts[0].contactPoint);
        }
    }

    void PhysicsService::PositionContraction() {
        for (const CollisionDef& collisionDef : mCollisions) {
            PhysicsBodyData& bodyA = mBodies[collisionDef.indexA];
            PhysicsBodyData& bodyB = mBodies[collisionDef.indexB];
            // Skip if both bodies are static
            if (bodyA.IsStatic() && bodyB.IsStatic()) {
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

            // Calculate total inverse mass
            float invMassSum = bodyA.InverseMass + bodyB.InverseMass;
            if (invMassSum <= 0.0f) {
                return; // Both have infinite mass
            }

            // Correction vector along the normal
            Math::Vector2f correction = collisionDef.penetration * (correctionMag / invMassSum) * percent;

            // Move bodies apart proportional to their inverse mass
            // Heavier objects move less, lighter objects move more
            bodyA.Position -= correction * bodyA.InverseMass;
            bodyB.Position += correction * bodyB.InverseMass;
        }
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
            body->ForceAccumulated += _force;
        }
    }

    void PhysicsService::ApplyForceAtPoint(BodyHandle _handle, Math::Vector2f _force, Math::Vector2f _worldPoint) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body && !body->IsStatic() && !body->bIsKinematic) {
            body->ForceAccumulated += _force;
            Math::Vector2f r = _worldPoint - body->Position;
            float torque     = Math::Vector2f::Cross2D(r, _force);
            body->TorqueAccumulated += torque;
        }
    }

    void PhysicsService::ApplyForceAtLocalPoint(BodyHandle _handle, Math::Vector2f _force, Math::Vector2f _localPoint) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body && !body->IsStatic() && !body->bIsKinematic) {
            body->ForceAccumulated += _force;
            Math::Vector2f rotatedPoint = _localPoint.GetRotated(body->Angle);
            float torque                = Math::Vector2f::Cross2D(rotatedPoint, _force);
            body->TorqueAccumulated += torque;
        }
    }

    void PhysicsService::ApplyImpulse(BodyHandle _handle, Math::Vector2f _impulse) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body && !body->IsStatic() && !body->bIsKinematic) {
            body->Velocity += _impulse * body->InverseMass;
        }
    }

    void PhysicsService::ApplyImpulseAtPoint(BodyHandle _handle, Math::Vector2f _impulse, Math::Vector2f _worldPoint) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body && !body->IsStatic() && !body->bIsKinematic) {
            body->Velocity += _impulse * body->InverseMass;
            Math::Vector2f r     = _worldPoint - body->Position;
            float angularImpulse = Math::Vector2f::Cross2D(r, _impulse);
            body->AngularVelocity += angularImpulse * body->InverseInertia;
        }
    }

    void PhysicsService::ApplyTorque(BodyHandle _handle, float _torque) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body && !body->IsStatic() && !body->bIsKinematic) {
            body->TorqueAccumulated += _torque;
        }
    }

    void PhysicsService::ApplyAngularImpulse(BodyHandle _handle, float _impulse) {
        PhysicsBodyData* body = GetBodyDataInternal(_handle);
        if (body && !body->IsStatic() && !body->bIsKinematic) {
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
