#include "Core/App.h"

#include "Core/AppWindow.h"
#include "Core/Event.h"
#include "Core/Random.h"
#include "ECS/ECSRegister.h"
#include "ECS/Systems/CollisionDetectionSystem.h"
#include "ECS/Systems/CollisionEventResolverSystem.h"
#include "ECS/Systems/LifeTimeSystem.h"
#include "ECS/Systems/PhysicsSystem.h"
#include "ECS/Systems/RenderSystem.h"
#include "ECS/Systems/RotationSystem.h"
#include "Game/IGameInstance.h"
#include "Game/SceneManager.h"
#include "Game/World.h"
#include "Input/Input.h"
#include "Platform/NativeFileSystem.h"
#include "Platform/PakFileSystem.h"
#include "Platform/VirtualFileManager.h"
#include "UI/Backends/SfmlImguiImpl.h"
#include "Umbra.h"
#include "Graphics/IRenderDevice.h"

namespace Umbra {

    App::App(SharedPtr<IGameInstance>& _gameInstance) {
        mGameInstance = _gameInstance;
    }

    App::~App() {}

    int App::Bootup() {
        if (Init()) {
            Run();
        }
        return Exit();
    }

    bool App::PreInit() {

        Logger::Config loggerConfig;
        loggerConfig.bEnable = true;
        Logger::Initialize(loggerConfig);

        mFileManager = std::make_unique<Platform::VirtualFileManager>();
        mFileManager->Mount("game/", "Asset/", std::make_shared<Platform::NativeFileSystem>());
        //  Initialize Platform layer

        /*
            @todo : HiRes Timer
            @todo : FileSystem
        */

        // Initialize Core Layer

        /*
            @todo 3rd party / dll ?
            @todo memoryManager
            @todo parser
            @todo Config <--parser
            @todo logger
            @todo math ?
            @todo RNG

        */


        EventBus::Initialize();

        Random::SetSeed(EngineTime::GetTimestamp(), EngineTime::GetTimestamp() / 2);
        // Initialize Resource Layer

        /*
            @todo ResourceManager
        */


        // Initialize Engine Layer
        Input::Initialize();
        mSceneManager = std::make_unique<SceneManager>();
        mUIManager    = std::make_unique<SfmlImguiImpl>();
        // GEngineStatics.

        return true;
    }

    bool App::Init() {

        bool preRequisiteSuccess = PreInit();
        if (preRequisiteSuccess) {
            if (mGameInstance == nullptr) {
                UMBRA_LOG_CRITICAL("GameInstance not Found, Exiting App");
            }
            mGameConfig               = mGameInstance->LoadGameConfig();
            GEngineStatics.GameConfig = &mGameConfig;
            mAppWindow                = std::make_shared<AppWindow>();
            bool windowSuccess        = mAppWindow->CreateWindow();
            if (!windowSuccess) {
                UMBRA_LOG_CRITICAL("Window Creation Failed, Exiting App!");
                return false;
            }
            EventBus::Subscribe<AppClosedEvent>(BIND_1P(this, &App::OnAppClosedEvent));
            mUIManager->Init(mAppWindow->GetRenderDevice()->GetNativeWindowHandle(), 800, 800);
            GEngineStatics.ImGuiBackend = mUIManager.get();
            UMBRA_LOG_INFO("App Initalized!");
            return true;
        }
        return false;
    }

    int App::Exit() {
        UMBRA_LOG_INFO("App Exiting!");
        mUIManager->Shutdown();
        mUIManager.reset();
        mSceneManager->ShutDown();
        mSceneManager.reset();
        mAppWindow->CloseWindow();
        mAppWindow.reset();
        mGameInstance->ShutDown();

        Input::Destroy();
        EventBus::Destroy();
        Logger::Destroy();
        return 0;
    }

    void App::Run() {

        // Initialize Gameplay framework;


        // InitializeECS();
        //  mWorld = new World();
        //  mWorld->SetECSRegister(&mWorldRegister);
        //  mGameInstance->SetCurrentWorld(mWorld);
        //  // LoadDefaultScene

        // // Start Game

        // // While App Running
        // // Poll Input
        // // Update World

        // // End Game
        // // ShutDown Gameplay framework;
        // // Unload Scenes

        try {
            mGameInstance->mSceneManager = mSceneManager.get();
            mGameInstance->mUIBackend    = mUIManager.get();
            mSceneManager->SetGameInstance(mGameInstance.get());
            mGameInstance->Initialize();
            float accumulatedDelta = 0.f;
            EngineTime::Reset();
            while (!bAppRequestExit) {

                float deltaTime = EngineTime::Tick();
                if (deltaTime == 0) {
                    UMBRA_LOG_CRITICAL("0 DT !! suffering from success");
                }
                if (!bAppPaused) {
                    if (deltaTime > mGameConfig.MaxPhysicsDeltaTime) { // Handle spiral of death
                        UMBRA_LOG_WARNING("Long frame detected %f", deltaTime);
                        deltaTime = mGameConfig.MaxPhysicsDeltaTime;
                    }
                    accumulatedDelta += deltaTime;
                    while (accumulatedDelta >= mGameConfig.FixedDeltaTime) {
                        accumulatedDelta -= mGameConfig.FixedDeltaTime;
                        OnFixedUpdate();
                    }
                }
                const float alpha = accumulatedDelta / mGameConfig.FixedDeltaTime;
                OnUpdate(alpha);
            }
        } catch (std::exception& e) {
            UMBRA_LOG_CRITICAL("Exception Faced ! %s", e.what());
        }
    }

    void App::OnUpdate(float _dt) {
        Input::Refresh();
        mAppWindow->Update();

        // Begin UI frame
        mUIManager->NewFrame(EngineTime::GetDeltaTime());

        // Render scene (includes OnUpdate and World rendering)
        mSceneManager->Render();

        // End UI frame
        mUIManager->Render();
    }

    void App::OnFixedUpdate() {
        mSceneManager->Simulate();
    }


    void App::OnAppClosedEvent(const AppClosedEvent& _event) {
        bAppRequestExit = true;
    }

    void App::InitializeECS() {

        // mWorldRegister.RegisterComponent<SpriteComponent>();
        // mWorldRegister.RegisterComponent<TransformComponent>();
        // mWorldRegister.RegisterComponent<LifeTimeComponent>();
        // mWorldRegister.RegisterComponent<RigidBodyComponent>();
        // mWorldRegister.RegisterComponent<CollisionBoxComponent>();
        // mWorldRegister.RegisterComponent<CollisionEventComponent>();

        // mRenderSystem = new RenderSystem(mAppWindow->GetRenderWindowHandle());
        // mWorldRegister.AddSystem(mRenderSystem);
        // mWorldRegister.AddSystem(new LifeTimeSystem());
        // mWorldRegister.AddSystem(new RotationSystem());
        // mWorldRegister.AddSystem(new PhysicsSystem());
        // mWorldRegister.AddSystem(new CollisionDetectionSystem());
        // mWorldRegister.AddSystem(new CollisionEventResolverSystem());
    }

    bool App::CreateWindow() {

        return true;
    }

    bool App::InitializeGameInstance() {
        // UM_ASSERT(mGameInstance != nullptr, "Game Instance not set!");
        // if (mGameInstance != nullptr) {
        //     mGameInstance->SetAppWindowRef(mAppWindow);
        //     mGameInstance->Initialize();
        //     return true;
        // } else {
        //     return false;
        // }
        return true;
    }

} // namespace Umbra
