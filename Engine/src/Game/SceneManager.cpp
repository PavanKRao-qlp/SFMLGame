#include "Game/SceneManager.h"

namespace Umbra {

    void SceneManager::Simulate() {
        if (mDeletedScene) {
            mDeletedScene->ShutDown();
            mDeletedScene.reset();
            mCurrentScene->OnBeginPlay();
        }
        if (mCurrentScene) {
            mCurrentScene->Simulate();
        }
    }

    void Umbra::SceneManager::Render() {
        if (mDeletedScene) {
            mDeletedScene->ShutDown();
            mDeletedScene.reset();
        }
        if (mCurrentScene) {
            if (!bCurrentSceneStarted) {
                bCurrentSceneStarted = true;
                mCurrentScene->OnBeginPlay();
            }
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
            mDeletedScene = std::move(mCurrentScene);
        }
        mCurrentScene = _scene->InsatiateCopy();
        mCurrentScene->Construct();
        bCurrentSceneStarted = false;
    }
    SceneManager::SceneManager() {}

    SceneManager::~SceneManager() {
        UMBRA_LOG_DEBUG("Scene Manager Destroyed!");
    }

    void SceneManager::SetGameInstance(IGameInstance* _GameInstance) {
        mGameInstance = _GameInstance;
    }

} // namespace Umbra
