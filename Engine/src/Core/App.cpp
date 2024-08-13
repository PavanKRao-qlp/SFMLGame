#include "Core/App.h"
#include "Diag/Logger.h"
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
        Logger::Log(LogType::Verbose,"App Booting Up!");
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
        Logger::Log(LogType::Verbose,"App Shuting Down!");
        return 0;
    }
}