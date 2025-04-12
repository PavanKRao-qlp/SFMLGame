#pragma once
#include "Game/IGameInstance.h"

namespace Umbra {
    class App {
    private:
        bool PreInit();
        bool Init();
        void Run();
        int Exit();

        void OnUpdate(float _dt);
        void OnFixedUpdate();
        void OnAppClosedEvent(const AppClosedEvent& _event);

        void InitializeECS();
        bool CreateWindow();
        bool InitializeGameInstance();

        bool bAppRequestExit = false;
        bool bAppPaused      = false;
        FGameConfig mGameConfig;
        // class World* mWorld;
        // class ECSRegister mWorldRegister;
        // class RenderSystem* mRenderSystem;
        SharedPtr<class AppWindow> mAppWindow;
        SharedPtr<class IGameInstance> mGameInstance;
        UniquePtr<class SceneManager> mSceneManager;
        UniquePtr<class ImGuiBackend> mUIManager;

    public:
        App(SharedPtr<IGameInstance>& _gameInstance);
        ~App();

        int Bootup();
    };

} // namespace Umbra
