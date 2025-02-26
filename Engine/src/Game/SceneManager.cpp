#include "Game/SceneManager.h"

namespace Umbra {

    void SceneManager::Simulate() {
        if (mCurrentScene) {
            mCurrentScene->Simulate();
        }
    }

    void Umbra::SceneManager::Render() {
        if (mCurrentScene) {
            mCurrentScene->Render();
        }
    }

    void SceneManager::AddScene(SharedPtr<Scene> _Scene) {
        mSceneMap.emplace(_Scene->GetSceneID(), _Scene);
        _Scene->SetSceneManager(this);
        _Scene->SetGameInstance(mGameInstance);
    }

    const SharedPtr<Scene>& SceneManager::GetCurrentScene() {
        return mCurrentScene;
    }

    void SceneManager::GoToScene(const String& _sceneId) {
        GoToScene(mSceneMap[_sceneId]);
    }


    void SceneManager::ShutDown() {
        if (mCurrentScene) {
            mCurrentScene->ShutDown();
            mCurrentScene.reset();
        }
        for (auto pair : mSceneMap) {
            pair.second->ShutDown();
        }
        mSceneMap.clear();
    }

    void SceneManager::GoToScene(SharedPtr<Scene>& _scene) {
        if (mCurrentScene) {
            mCurrentScene->ShutDown();
            mCurrentScene.reset();
        }
        mCurrentScene = _scene->InsatiateCopy();
        mCurrentScene->Construct();
        mCurrentScene->OnBeginPlay();
    }
    SceneManager::SceneManager() {}

    SceneManager::~SceneManager() {
        UMBRA_LOG_WARNING("Scene Manager Destroyed!");
    }

    void SceneManager::SetGameInstance(IGameInstance* _GameInstance) {
        mGameInstance = _GameInstance;
    }

} // namespace Umbra
