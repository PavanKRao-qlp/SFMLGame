#include "Game/Scene.h"

#include "Game/IGameInstance.h"
namespace Umbra {
    Scene::Scene() {}

    Scene::~Scene() {
        UMBRA_LOG_DEBUG("Scene Destroyed");
    }

    void Scene::Construct() {
        UMBRA_LOG_INFO("Construct Scene %s", mSceneIdentifier.c_str());
        mWorld = std::make_unique<World>();
        mWorld->InitializeCoreSystems();

        // add Camera Entity
        mCameraEntity = mWorld->CreateEntity();
        mWorld->AddComponent<TransformComponent>(
            mCameraEntity, TransformComponent(Math::Vector2f(0, 0), Math::Vector2f(0, 0)));
        CameraComponent cameraComponent;
        cameraComponent.SetOrthographicSize(150 / 2);
        cameraComponent.SetActive(true);
        mWorld->AddComponent<CameraComponent>(mCameraEntity, cameraComponent);
        Initialize();
        bLoaded = true;
    }

    void Scene::Render() {
        GEngineStatics.AppWindowPtr->GetRenderWindowHandle()->clear(sf::Color::Black);
        mGameInstance->GetUIManager().NewFrame(EngineTime::GetDeltaTime());
        this->OnUpdate();
        mWorld->Update();
        mGameInstance->GetUIManager().Render();
        mWorld->Render();
    }

    void Scene::Simulate() {
        RenderSystem::DebugDrawCache.clear();
        this->OnFixedUpdated();
        mWorld->Simulate();
    }

    void Scene::ShutDown() {
        UMBRA_LOG_INFO("ShutDown Scene %s", mSceneIdentifier.c_str());
        mWorld.reset();
    }

    bool Scene::IsLoaded() {
        return bLoaded;
    }

    const String& Scene::GetSceneID() {
        return mSceneIdentifier;
    }

    World* Scene::GetWorld() {
        return mWorld.get();
    }

    void Scene::SetGameInstance(IGameInstance* _gameInstance) {
        mGameInstance = _gameInstance;
    }

    void Scene::SetSceneManager(SceneManager* _sceneManager) {
        mSceneManager = _sceneManager;
    }

    IGameInstance& Scene::GetGameInstance() {
        return *mGameInstance;
    }

    SceneManager& Scene::GetSceneManager() {
        return *mSceneManager;
    }

    EntityID Scene::GetCameraEntity() {
        return mCameraEntity;
    }

    Scene::Scene(const Scene& _scene) {
        mSceneIdentifier = _scene.mSceneIdentifier;
        mSceneManager    = _scene.mSceneManager;
        mGameInstance    = _scene.mGameInstance;
    }

    void Scene::SetSceneId(String _sceneId) {
        mSceneIdentifier = _sceneId;
    }
} // namespace Umbra
