#pragma once
#include "Core/AppWindow.h"

namespace D2D
{
    class App
    {
    private:
        bool Init();
        void Run();
        int Exit();

        bool mAppRunning = false;
        class AppWindow *mAppWindow;
    public:
        App(/* args */);
        ~App();
        int Bootup();
    };
}