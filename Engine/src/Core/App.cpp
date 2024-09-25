#include "Core/App.h"

#include "ECS/Systems/LifeTimeSystem.h"
#include "ECS/Systems/RotationSystem.h"
#include "Input/Input.h"
#include "Umbra.h"

namespace Umbra {
    const float fixedDt = 1.f / 60;
    App::App(IGameInstance* gameInstance) {
        mGameInstance = gameInstance;
    }

    App::~App() {}

    int App::Bootup() {
        Logger::Log(LogType::Verbose, "App Booting Up!");
        if (Init()) {
            Run();
        }
        return Exit();
    }

    bool App::Init() {
        EventBus::Subscribe<AppClosedEvent>(BIND_1P(this, &App::OnAppWindowClosed));
        Input Input;
        mAppWindow = new AppWindow();
        mAppWindow->CreateWindow();
        bAppRunning = true;

        mRenderSystem = new RenderSystem(mAppWindow->GetRenderWindowHandle());
        mWorldRegister.RegisterComponent<SpriteComponent>();
        mWorldRegister.RegisterComponent<TransformComponent>();
        mWorldRegister.RegisterComponent<LifeTimeComponent>();


        mWorldRegister.AddSystem(mRenderSystem);
        mWorldRegister.AddSystem(new RotationSystem());
        mWorldRegister.AddSystem(new LifeTimeSystem());

        UM_ASSERT(mGameInstance != nullptr, "Game Instance not set!");
        if (mGameInstance != nullptr) {
            mGameInstance->SetECSRegister(&mWorldRegister);
            mGameInstance->Initialize();
        } else {
            return false;
        }
        return true;
    }

    void App::Run() {
        float dt     = 0;
        double accDt = 0;
        if (mGameInstance != nullptr) {
            mGameInstance->OnBeginPlay();
        }
        while (bAppRunning) {
            dt = EngineTime::Tick();
            accDt += dt;
            OnUpdate(dt);
            while (accDt >= fixedDt) {
                OnFixedUpdate();
                accDt -= fixedDt;
            }
        }
        if (mGameInstance != nullptr) {
            mGameInstance->OnEndPlay();
        }
    }

    void App::OnUpdate(float _dt) {
        mAppWindow->Update();
        if (mGameInstance != nullptr) {
            mGameInstance->OnUpdate(_dt);
            mWorldRegister.Update();
        }
        Input::Update();
    }

    void App::OnFixedUpdate() {}

    int App::Exit() {
        Logger::Log(LogType::Verbose, "App Shuting Down!");
        EventBus::Flush();
        delete mAppWindow;
        return 0;
    }

    void App::OnAppWindowClosed(const AppClosedEvent& _event) {
        bAppRunning = false;
        mAppWindow->CloseWindow();
    }
} // namespace Umbra
