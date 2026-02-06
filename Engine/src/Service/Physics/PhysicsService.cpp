#include "Service/Physics/PhysicsService.h"

#include "Math/MathUtils.h"

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
        body.bAffectedByGravity  = _def.bAffectedByGravity;
        body.bIsKinematic        = _def.bIsKinematic;
        body.bIsActive           = true;
        body.UserData            = _def.UserData;
        body.Generation          = generation;

        // Set mass
        body.SetMass(_def.Mass);
        body.SetInertia(_def.Inertia);

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

    // ============== Internal Access ==============

    PhysicsBodyData* PhysicsService::GetBodyData(BodyHandle _handle) {
        return GetBodyDataInternal(_handle);
    }

    const PhysicsBodyData* PhysicsService::GetBodyData(BodyHandle _handle) const {
        return GetBodyDataInternal(_handle);
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
