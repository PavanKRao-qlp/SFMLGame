#include "Service/ServiceLocator.h"

namespace Umbra {

    UniquePtr<PhysicsService> ServiceLocator::sPhysicsService = nullptr;
    UniquePtr<RenderService> ServiceLocator::sRenderService   = nullptr;

    void ServiceLocator::Initialize() {
        // Create default physics service if not already set
        if (sPhysicsService == nullptr) {
            PhysicsServiceConfig config;
            sPhysicsService = std::make_unique<PhysicsService>(config);
        }

        // Create default render service if not already set
        if (sRenderService == nullptr) {
            sRenderService = std::make_unique<RenderService>();
        }
    }

    void ServiceLocator::Shutdown() {
        sRenderService.reset();
        sPhysicsService.reset();
    }

} // namespace Umbra
