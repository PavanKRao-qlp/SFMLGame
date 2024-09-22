#pragma once
#include "Core/AppWindow.h"
#include "Core/Event.h"
#include "ECS/ECSRegister.h"
#include "Game/IGameInstance.h"
#include "ECS/Systems/RenderSystem.h"

namespace D2D
{
    class App
    {
    private:
        bool Init();
        void Run();
        int Exit();
        
        void OnUpdate(float _dt);
        void OnFixedUpdate();
        void OnAppWindowClosed(const AppClosedEvent& _event);

        bool mAppRunning = false;
        class AppWindow *mAppWindow;
        class ECSRegister mWorldRegister;
        class RenderSystem* mRenderSystem;
        class IGameInstance* mGameInstance;

    public:
        App(IGameInstance* gameInstance);
        ~App();
        int Bootup();
    };
}