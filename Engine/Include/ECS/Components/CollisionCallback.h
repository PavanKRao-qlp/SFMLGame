#pragma once
#include "ECS/Component.h"
#include "ECS/ECSConfig.h"
#include "Service/Physics/Collision.h"

#include <functional>

namespace Umbra {

    struct CollisionCallbackComponent : public Component {
        using Callback     = std::function<void(EntityID, EntityID, const CollisionEvent&)>;
        using ExitCallback = std::function<void(EntityID, EntityID)>;

        Callback OnCollisionEnter;
        Callback OnCollisionStay;
        ExitCallback OnCollisionExit;

        Callback OnTriggerEnter;
        Callback OnTriggerStay;
        ExitCallback OnTriggerExit;
    };

} // namespace Umbra
