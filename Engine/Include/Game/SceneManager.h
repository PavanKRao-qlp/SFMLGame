#pragma once
#include "EnginePCH.h"
#include "Game/Scene.h"
#include "Game/SceneContext.h"
namespace Umbra {
    class SceneManager {
    private:
        /* data */
    public:
        void Simulate();
        void Render();
        void AddScene(String _sceneId, SharedPtr<Scene> _scene);
        const SharedPtr<Scene>& GetCurrentScene();

        // Transition by scene pointer (optional context forwarded to the incoming scene).
        void GoToScene(SharedPtr<Scene>& _scene, SceneContext _context = {});
        // Transition by registered scene ID (optional context forwarded to the incoming scene).
        void GoToScene(const String& _sceneId, SceneContext _context = {});

        // Returns the context that was passed to the most recent GoToScene call.
        const SceneContext& GetContext() const;

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
        SceneContext mContext;
    };
} // namespace Umbra
