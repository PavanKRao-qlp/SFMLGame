#pragma once
#include "EnginePCH.h"
#include "Game/Scene.h"
namespace Umbra {
    class SceneManager {
    private:
        /* data */
    public:
        void Simulate();
        void Render();
        void AddScene(SharedPtr<Scene> _scene);
        const SharedPtr<Scene>& GetCurrentScene();
        void GoToScene(SharedPtr<Scene>& _scene);
        void GoToScene(const String& _sceneId);
        void ShutDown();

        SceneManager(/* args */);
        ~SceneManager();
        void SetGameInstance(IGameInstance* _GameInstance);

    private:
        SharedPtr<Scene> mCurrentScene;
        UMap<String, SharedPtr<Scene>> mSceneMap;
        IGameInstance* mGameInstance;
        SharedPtr<Scene> mDeletedScene;
        bool bCurrentSceneStarted = false;
    };
} // namespace Umbra
