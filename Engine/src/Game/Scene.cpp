#include "Game/Scene.h"

#include "Game/IGameInstance.h"
#include "Graphics/Color.h"
namespace Umbra {
    Scene::Scene() {}

    Scene::~Scene() {
        UMBRA_LOG_DEBUG("Scene Destroyed");
    }

    void Scene::Construct() {
        UMBRA_LOG_INFO("Construct Scene %s", mSceneIdentifier.c_str());
        mWorld = std::make_unique<World>();
        mWorld->InitializeCoreSystems();

        // Note: Camera is no longer auto-created.
        // Scenes should call CreateDefaultCamera() in Initialize() or BeginPlay() if needed.
        Initialize();
        bLoaded = true;
    }

    void Scene::Render() {
        GEngineStatics.AppWindowPtr->GetRenderDevice()->Clear(Color::Black);
        this->OnUpdate();
        mWorld->Update();
        mWorld->Render();
    }

    void Scene::Simulate() {
        this->OnFixedUpdate();
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

    void Scene::SetMainCamera(EntityID _camera) {
        mCameraEntity = _camera;
    }

    EntityID Scene::CreateDefaultCamera(float _orthographicSize) {
        EntityID camera = mWorld->CreateEntity();
        mWorld->AddComponent<TransformComponent>(
            camera, TransformComponent(Math::Vector2f(0, 0), Math::Vector2f(0, 0)));
        CameraComponent cameraComponent;
        cameraComponent.SetOrthographicSize(_orthographicSize);
        cameraComponent.SetActive(true);
        mWorld->AddComponent<CameraComponent>(camera, cameraComponent);
        mCameraEntity = camera;  // Set as main camera
        return camera;
    }
} // namespace Umbra
