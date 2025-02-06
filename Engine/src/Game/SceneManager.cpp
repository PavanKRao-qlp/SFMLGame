#include "Game/SceneManager.h"
namespace Umbra {
    void SceneManager::Simulate() {
        if (mCurrentScene) {
            mCurrentScene->FixedUpdate();
        }
    }
    void Umbra::SceneManager::Render() {
        if (mCurrentScene) {
            mCurrentScene->Update();
        }
    }

    void SceneManager::AddScene(SharedPtr<Scene> _Scene) {
        mSceneMap.emplace(_Scene->GetSceneID(), _Scene);
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
            mCurrentScene = nullptr;
        }
        for (auto pair : mSceneMap) {
            pair.second->ShutDown();
        }
        mSceneMap.clear();
    }

    void SceneManager::GoToScene(SharedPtr<Scene>& _scene) {
        if (mCurrentScene) {
            // mCurrentScene->Unload();
        }
        mCurrentScene = _scene;
        mCurrentScene->Construct();
        mCurrentScene->OnBeginPlay();
    }
    SceneManager::SceneManager() {}

    SceneManager::~SceneManager() {}
} // namespace Umbra
