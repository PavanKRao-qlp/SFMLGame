#include "Service/ServiceLocator.h"

namespace Umbra {

    UniquePtr<PhysicsService> ServiceLocator::sPhysicsService = nullptr;

    void ServiceLocator::Initialize() {
        // Create default physics service if not already set
        if (sPhysicsService == nullptr) {
            PhysicsServiceConfig config;
            sPhysicsService = std::make_unique<PhysicsService>(config);
        }
    }

    void ServiceLocator::Shutdown() {
        sPhysicsService.reset();
    }

} // namespace Umbra
