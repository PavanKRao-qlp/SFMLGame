#pragma once
#include "ECS/Component.h"
#include "Math/Vector.h"

namespace Umbra {
    struct PhysicsBodyComponent : public Component {
    public:
        // * Speed of movement of Body
        double mVelocity = 0;
        // * Acceleration to velocity
        double mAcceleration = 0;
        // * Holds (1/mass) of body as 0 mass causes divide by zero invalidity but 0 inverse mass can be consider an
        // object of infinite mass
        double mInverseMass;
    };
} // namespace Umbra
