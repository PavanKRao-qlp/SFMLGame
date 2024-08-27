#pragma once
#include "Core/AppWindow.h"
#include "Core/Event.h"
#include "ECS/ECSRegister.h"
#include "ECS/Systems/RenderSystem.h"

namespace D2D
{
    class App
    {
    private:
        bool Init();
        void Run();
        int Exit();
        void OnAppWindowClosed(const AppClosedEvent& Event);

        bool mAppRunning = false;
        class AppWindow *mAppWindow;
        class ECSRegister mWorldRegister;
        class RenderSystem* mRenderSystem;

    public:
        App(/* args */);
        ~App();
        int Bootup();
    };
}