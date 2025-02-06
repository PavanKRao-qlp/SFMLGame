#include "Game/Scene.h"

#include "Game/IGameInstance.h"

namespace Umbra {
    Scene::Scene() {}

    Scene::~Scene() {
        UMBRA_LOG_DEBUG("Scene Destroyed");
    }

    void Scene::Construct() {
        mWorld = std::make_unique<World>();
        mWorld->InitializeCoreSystems();
        // add Camera Entity
        Initialize();
        bLoaded = true;
    }


    void Scene::Update() {
        this->OnUpdate();
        UMBRA_LOG_WARNING("OnUpdate");
        mWorld->Update();
    }
    void Scene::FixedUpdate() {}

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
