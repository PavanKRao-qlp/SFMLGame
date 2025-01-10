#include "Core/App.h"

#include "ECS/Systems/CollisionDetectionSystem.h"
#include "ECS/Systems/CollisionEventResolverSystem.h"
#include "ECS/Systems/LifeTimeSystem.h"
#include "ECS/Systems/PhysicsSystem.h"
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
        // @todo CheckSystemCompatable();
        //  @todo  Initialize Memory Pool
        //  @todo  Initialize AssetRegister
        //  @todo  Initialize SoundSystem
        //  @todo  Initialize Save Systems

        // Initialize Core Module
        Random::SetSeed(EngineTime::GetTimestampMS(), EngineTime::GetTimestampMS() / 2);

        // Initialize Window View
        if (!CreateWindow()) {
            return false;
        }
        Input Input; // refactor
        EventBus::Subscribe<AppClosedEvent>(BIND_1P(this, &App::OnAppWindowClosed));

        // Initialize Core Engine Services

        // Check and Initialize GameInstance
        if (!InitializeGameInstance()) {
            return false;
        }
        return true;
    }

    void App::Run() {

        // Initialize Gameplay framework;

        InitializeECS();
        mWorld = new World();
        mWorld->SetECSRegister(&mWorldRegister);
        mGameInstance->SetCurrentWorld(mWorld);
        // LoadDefaultScene

        // Start Game

        // While App Running
        // Poll Input
        // Update World

        // End Game
        // ShutDown Gameplay framework;
        // Unload Scenes

        bAppRunning  = true;
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
            mWorldRegister.Update();
            mGameInstance->OnUpdate(_dt);
        }
        Input::Update();
    }

    void App::OnFixedUpdate() {}

    int App::Exit() {
        Logger::Log(LogType::Verbose, "App Shuting Down!");
        // ShutDown GameInstance
        // Shutdown Core Engine Services
        // Close Window in case
        // Core module shutdown
        EventBus::Flush();
        delete mAppWindow;
        return 0;
    }

    void App::OnAppWindowClosed(const AppClosedEvent& _event) {
        bAppRunning = false;
        mAppWindow->CloseWindow();
    }

    void App::InitializeECS() {

        mWorldRegister.RegisterComponent<SpriteComponent>();
        mWorldRegister.RegisterComponent<TransformComponent>();
        mWorldRegister.RegisterComponent<LifeTimeComponent>();
        mWorldRegister.RegisterComponent<RigidBodyComponent>();
        mWorldRegister.RegisterComponent<CollisionBoxComponent>();
        mWorldRegister.RegisterComponent<CollisionEventComponent>();

        mRenderSystem = new RenderSystem(mAppWindow->GetRenderWindowHandle());
        mWorldRegister.AddSystem(mRenderSystem);
        mWorldRegister.AddSystem(new LifeTimeSystem());
        mWorldRegister.AddSystem(new RotationSystem());
        mWorldRegister.AddSystem(new PhysicsSystem());
        mWorldRegister.AddSystem(new CollisionDetectionSystem());
        mWorldRegister.AddSystem(new CollisionEventResolverSystem());
    }

    bool App::CreateWindow() {
        mAppWindow = new AppWindow();
        return mAppWindow->CreateWindow();
    }

    bool App::InitializeGameInstance() {
        UM_ASSERT(mGameInstance != nullptr, "Game Instance not set!");
        if (mGameInstance != nullptr) {
            mGameInstance->SetAppWindowRef(mAppWindow);
            mGameInstance->Initialize();
            return true;
        } else {
            return false;
        }
    }

} // namespace Umbra
