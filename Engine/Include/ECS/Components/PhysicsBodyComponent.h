#pragma once
#include "ECS/Component.h"
#include "Math/Vector.h"

namespace Umbra {
    struct PhysicsBodyComponent : public Component {
    public:
        // * Speed of movement of Body
        Math::Vector2f mVelocity         = 0;
        Math::Vector2f mForceAccumulated = 0;
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
        // * Acceleration to velocity
        Math::Vector2f mAcceleration = 0;
        // * Should gravity be applied to the body
        bool bAffectedByGravity = true;
    };
} // namespace Umbra
