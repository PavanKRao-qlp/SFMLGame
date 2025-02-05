#pragma once
#include "ECS/ECSRegister.h"
#include "ECS/Systems/RenderSystem.h"
#include "Umbra.h"

namespace Umbra {
    class World {
    public:
        void InitializeCoreSystems();
        // void SetECSRegister(ECSRegister* _worldRegister);
        // ECSRegister* GetRegister();
        // void FlushWorld();
        World();

    private:
        ECSRegister* mWorldRegister;
        SharedPtr<RenderSystem> mRenderSystem;
    };

} // namespace Umbra
