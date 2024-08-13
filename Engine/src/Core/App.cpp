#include "Core/App.h"

namespace D2D
{

    App::App()
    {
    }

    App::~App()
    {
    }

    int App::Bootup()
    {
        if (Init())
        {
            Run();
        }
        return Exit();
    }

    bool App::Init()
    {
        return true;
    }

    void App::Run()
    {
        while (mAppRunning)
        {
            /* code */
        }
    }

    int App::Exit()
    {
        return 0;
    }
}