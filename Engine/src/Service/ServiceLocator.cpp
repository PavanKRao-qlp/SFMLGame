#include "Service/ServiceLocator.h"

namespace Umbra {

    UniquePtr<PhysicsService> ServiceLocator::sPhysicsService = nullptr;
    UniquePtr<RenderService>  ServiceLocator::sRenderService  = nullptr;
    UniquePtr<JobSystem>      ServiceLocator::sJobSystem       = nullptr;

    void ServiceLocator::Initialize() {
        // Create default job system first — other services may use it in the future
        if (sJobSystem == nullptr) {
            sJobSystem = std::make_unique<JobSystem>();  // 0 = hardware_concurrency - 1
        }

        // Create default physics service if not already set
        if (sPhysicsService == nullptr) {
            PhysicsServiceConfig config;
            sPhysicsService = std::make_unique<PhysicsService>(config);
        }

        // Wire up the job system so physics integration runs in parallel
        sPhysicsService->SetJobSystem(sJobSystem.get());

        // Create default render service if not already set
        if (sRenderService == nullptr) {
            sRenderService = std::make_unique<RenderService>();
        }
    }

    void ServiceLocator::Shutdown() {
        sRenderService.reset();
        sPhysicsService.reset();
        // Shut down the job system last — gracefully drains remaining jobs
        if (sJobSystem) {
            sJobSystem->Shutdown();
            sJobSystem.reset();
        }
    }

} // namespace Umbra
