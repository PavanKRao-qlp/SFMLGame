#include "Core/App.h"
#include "Core/Event.h"
#include "Input/Input.h"
#include "Diag/Logger.h"
#include "ECS/Systems/RotationSystem.h"

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

        mWorldRegister.RegisterComponent<SpriteComponent>();
        mWorldRegister.RegisterComponent<TransformComponent>();

        EntityID ballA = mWorldRegister.CreateEntity();
        mWorldRegister.AddComponent<TransformComponent>(ballA, TransformComponent(300, 300));
        mWorldRegister.AddComponent<SpriteComponent>(ballA, sf::Color::Red);
        mWorldRegister.GetComponent<TransformComponent>(ballA)->x = 400;

        EntityID ballB = mWorldRegister.CreateEntity();
        mWorldRegister.AddComponent<TransformComponent>(ballB, TransformComponent(650, 100));
        mWorldRegister.AddComponent<SpriteComponent>(ballB, sf::Color::Blue);

        mRenderSystem = new RenderSystem(mAppWindow->GetRenderWindowHandle());
        mWorldRegister.AddSystem(mRenderSystem);
        mWorldRegister.AddSystem(new RotationSystem());

        return true;
    }

    void App::Run()
    {
        while (mAppRunning)
        {
            mAppWindow->Update();

            // mAppWindow->ClearDisplay();
            mWorldRegister.Update();
            // mAppWindow->RefreshDisplay();
            if (Input::GetMouseButtonUp(D2D::Mouse::Left))
            {
                EntityID ballB = mWorldRegister.CreateEntity();
                mWorldRegister.AddComponent<TransformComponent>(ballB, TransformComponent(Input::GetMousePositionX(), Input::GetMousePositionY()));
                mWorldRegister.AddComponent<SpriteComponent>(ballB, sf::Color::Green);
            }
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
