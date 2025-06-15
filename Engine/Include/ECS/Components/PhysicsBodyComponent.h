#pragma once
#include "ECS/Component.h"
#include "Math/Vector.h"

namespace Umbra {
    struct PhysicsBodyComponent : public Component {
    public:
        // * Applies force at the center of mass e.i no rotational force
        void ApplyForce(Math::Vector2f _force) {
            mForceAccumulated += _force;
        }
        // * Applies force at point relative to center of mass
        void ApplyForceAtLocal(Math::Vector2f _force, float _rotation, Math::Vector2f _position) {
            double torque = Math::Vector2f::Cross2D(_position.GetRotated(_rotation), _force);
            mTorqueAccumulated += torque;
            mForceAccumulated += _force;
        }
        // *  Applies Torque around center of mass
        void ApplyTorque(double _torque) {
            mTorqueAccumulated += _torque;
        }
        // * Speed of movement of Body
        Math::Vector2f mVelocity         = 0;
        double mAngularVelocity          = 0;
        Math::Vector2f mForceAccumulated = 0;
        double mTorqueAccumulated        = 0;
        inline bool IsStatic() {
            return mInverseMass <= 0;
        }

        inline void SetMass(double _mass) {
            if (_mass > 0) {
                mInverseMass = 1 / _mass;
            } else {
                mInverseMass = 0;
            }
        }
        // * Holds (1/mass) of body as 0 mass causes divide by zero invalidity but 0 inverse mass can be consider an
        // object of infinite mass
        double mInverseMass = 0;
        //
        double mInverseInertia = 0;
        // * Acceleration to velocity
        Math::Vector2f mAcceleration = 0;
        // * Acceleration to velocity
        double mAngularAcceleration = 0;
        // * Should gravity be applied to the body
        bool bAffectedByGravity = true;
        // * How bouncy is the surface? A value of 0 will not bounce. A value of 1 will bounce without any loss of
        // energy. Value between 0-1
        float mCofOfRestitution = 0;
    };
} // namespace Umbra
