#include "Game/World.h"

#include "ECS/Components/SpriteQuad.h"
#include "ECS/Systems/RenderSystem.h"


namespace Umbra {

    // void World::SetECSRegister(ECSRegister* _worldRegister) {
    //     mWorldRegister = _worldRegister;
    // }

    // ECSRegister* World::GetRegister() {
    //     return mWorldRegister;
    // }

    // void World::FlushWorld() {
    //     mWorldRegister->FlushRegister();
    // }

    void World::InitializeCoreSystems() {
        mWorldRegister->RegisterComponent<SpriteComponent>();
        mRenderSystem = std::make_shared<RenderSystem>();
        mWorldRegister->AddSystem(mRenderSystem);
    }

    World::World() {
        mWorldRegister = std::make_shared<ECSRegister>();
    }

} // namespace Umbra
