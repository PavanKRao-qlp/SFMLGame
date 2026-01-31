#include "Game/SceneManager.h"

#include "Game/IGameInstance.h"
namespace Umbra {

    void SceneManager::Simulate() {
        if (mDeletedScene) {
            mDeletedScene->ShutDown();
            mDeletedScene.reset();
        }
        if (bCurrentSceneStarted && mCurrentScene) {
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

    void SceneManager::AddScene(String _sceneId, SharedPtr<Scene> _Scene) {
        mSceneMap.emplace(_sceneId, _Scene);
        _Scene->SetSceneManager(this);
        _Scene->SetSceneId(_sceneId);
        _Scene->SetGameInstance(mGameInstance);
    }

    const SharedPtr<Scene>& SceneManager::GetCurrentScene() {
        return mCurrentScene;
    }

    void SceneManager::GoToScene(const String& _sceneId) {
        if (mSceneMap.find(_sceneId) == mSceneMap.end()) {
            UMBRA_LOG_CRITICAL("Trying To Load Unkown Scene!:%s", _sceneId.c_str());
            mGameInstance->QuitApplication();
        }
        {
            GoToScene(mSceneMap[_sceneId]);
        }
    }


    void SceneManager::ShutDown() {
        if (mCurrentScene) {
            // Call OnEndPlay if scene was started
            if (bCurrentSceneStarted) {
                mCurrentScene->OnEndPlay();
            }
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
            UMBRA_LOG_INFO("GoToScene exiting %s", mCurrentScene->GetSceneID().c_str());
            // Call OnEndPlay before transitioning
            if (bCurrentSceneStarted) {
                mCurrentScene->OnEndPlay();
            }
            // Transfer current scene to deletion queue (will be cleaned up in Render/Simulate)
            mDeletedScene = mCurrentScene;
            mCurrentScene.reset();
        }

        // Create new instance from template
        mCurrentScene = _scene->InstantiateCopy();
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
