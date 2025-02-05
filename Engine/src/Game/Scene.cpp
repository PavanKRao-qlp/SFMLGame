#include "Game/Scene.h"

#include "Game/IGameInstance.h"

namespace Umbra {
    Scene::Scene() {}

    Scene::~Scene() {
        UMBRA_LOG_DEBUG("Scene Destroyed");
    }

    void Scene::Construct() {
        mWorld = std::make_shared<World>();
        mWorld->InitializeCoreSystems();
        // add Camera Entity
        Initialize();
        bLoaded = true;
    }

    void Scene::OnBeginPlay() {}


    void Scene::OnFixedUpdated() {}

    void Scene::OnUpdate() {}

    void Scene::OnEndPlay() {}

    void Scene::ShutDown() {}

    bool Scene::IsLoaded() {
        return bLoaded;
    }

    const String& Scene::GetSceneID() {
        return mSceneIdentifier;
    }
    const World& Scene::GetWorld() {
        return *mWorld;
    }

    IGameInstance* Scene::GetGameInstance() {
        return mGameInstance.get();
    }

} // namespace Umbra
