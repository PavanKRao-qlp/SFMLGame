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
        Logger::Log(LogType::Verbose, "App Booting Up!");
        if (Init())
        {
            Run();
        }
        return Exit();
    }

    bool App::Init()
    {
        mAppWindow = new AppWindow();
        mAppWindow->CreateWindow();
        return true;
    }

    void App::Run()
    {
        while (!mAppWindow->bWindowClosed)
        {

            mAppWindow->Update();
            mAppWindow->ClearDisplay();
            // for each spriteComponent S Get tranform T
            {
                // Renderer->Draw(S)
            }
            mAppWindow->RefreshDisplay();
        }
    }

    int App::Exit()
    {
        Logger::Log(LogType::Verbose, "App Shuting Down!");
        mAppWindow->CloseWindow();
        delete mAppWindow;
        return 0;
    }
}