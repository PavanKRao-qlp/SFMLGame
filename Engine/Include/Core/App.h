#pragma once
#include "Core/AppWindow.h"
#include "Core/Event.h"

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

    public:
        App(/* args */);
        ~App();
        int Bootup();
    };
}