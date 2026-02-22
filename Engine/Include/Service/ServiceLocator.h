#pragma once
#include "EnginePCH.h"
#include "Service/Physics/PhysicsService.h"
#include "Service/Render/RenderService.h"
#include "Thread/JobSystem.h"

namespace Umbra {

    /// @brief Global service locator for engine services
    /// Provides centralized access to services without tight coupling
    class ServiceLocator {
    public:
        /// @brief Initialize all services (called during engine startup)
        static void Initialize();

        /// @brief Shutdown all services (called during engine shutdown)
        static void Shutdown();

        /// @brief Get the physics service
        static PhysicsService* GetPhysicsService() { return sPhysicsService.get(); }

        /// @brief Register a custom physics service (for testing or custom implementations)
        static void RegisterPhysicsService(UniquePtr<PhysicsService> _service) {
            sPhysicsService = std::move(_service);
        }

        /// @brief Get the render service
        static RenderService* GetRenderService() { return sRenderService.get(); }

        /// @brief Register a custom render service
        static void RegisterRenderService(UniquePtr<RenderService> _service) {
            sRenderService = std::move(_service);
        }

        /// @brief Get the job system (thread pool)
        static JobSystem* GetJobSystem() { return sJobSystem.get(); }

        /// @brief Register a custom job system
        static void RegisterJobSystem(UniquePtr<JobSystem> _jobSystem) {
            sJobSystem = std::move(_jobSystem);
        }

    private:
        static UniquePtr<PhysicsService> sPhysicsService;
        static UniquePtr<RenderService>  sRenderService;
        static UniquePtr<JobSystem>      sJobSystem;
    };

} // namespace Umbra
