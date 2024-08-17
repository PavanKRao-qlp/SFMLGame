#include "Core/App.h"
#include "Core/Event.h"
#include "Input/Input.h"
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
        EventBus::Subscribe<AppClosedEvent>(BIND_1P(this, &App::OnAppWindowClosed));
        Input Input;
        mAppWindow = new AppWindow();
        mAppWindow->CreateWindow();
        mAppRunning = true;
        return true;
    }

    void App::Run()
    {
        while (mAppRunning)
        {

            mAppWindow->Update();
            mAppWindow->ClearDisplay();
            // for each spriteComponent S Get transform T
            {
                // Renderer->Draw(S)
            }
            printf("%d %d \n", Input::GetMousePositionX(), Input::GetMousePositionY());
            if(Input::GetKey(KeyBoard::Keycode::Left)){
                Logger::Log(LogType::Verbose, "Left Pressed");
            } 
            if(Input::GetKeyDown(KeyBoard::Keycode::Left)){
                Logger::Log(LogType::Verbose, "Left Held");
            }
            mAppWindow->RefreshDisplay();
        }
    }

    int App::Exit()
    {
        Logger::Log(LogType::Verbose, "App Shuting Down!");
        EventBus::Flush();
        delete mAppWindow;
        return 0;
    }

    void App::OnAppWindowClosed(const AppClosedEvent &Event)
    {
        mAppRunning = false;
        mAppWindow->CloseWindow();
    }
}
