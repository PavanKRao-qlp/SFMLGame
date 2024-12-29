#pragma once
#include "Core/AppWindow.h"
#include "Core/Event.h"
#include "Core/Random.h"
#include "ECS/ECSRegister.h"
#include "ECS/Systems/RenderSystem.h"
#include "Game/IGameInstance.h"

namespace Umbra {
    class App {
    private:
        bool Init();
        void Run();
        int Exit();

        void OnUpdate(float _dt);
        void OnFixedUpdate();
        void OnAppWindowClosed(const AppClosedEvent& _event);

        void InitializeECS();
        bool CreateWindow();
        bool InitializeGameInstance();

        bool bAppRunning = false;
        class AppWindow* mAppWindow;
        class ECSRegister mWorldRegister;
        class RenderSystem* mRenderSystem;
        class IGameInstance* mGameInstance;

    public:
        App(IGameInstance* gameInstance);
        ~App();
        int Bootup();
    };
} // namespace Umbra
