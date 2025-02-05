#pragma once
#include "Core/AppWindow.h"
#include "Core/Clock.h"
#include "ECS/ECSRegister.h"
#include "Game/SceneManager.h"
#include "Game/World.h"
#include "Math/Vector.h"
#include "Umbra.h"

#include "SFML/Graphics.hpp"
namespace Umbra {
    struct FGameConfig {
        Math::Vector2i WindowSize = Math::Vector2i(800, 800);
        bool bFullScreen;
        float FixedDeltaTime      = 0.02f;
        float MaxPhysicsDeltaTime = 0.1f;
    };

    class IGameInstance {
    public:
        IGameInstance()  = default;
        ~IGameInstance() = default;
        virtual FGameConfig& LoadGameConfig();
        virtual void Initialize() = 0;
        virtual void ShutDown()   = 0;
        // virtual void OnUpdate(float dt) = 0;
        // virtual void OnBeginPlay()      = 0;
        // virtual void OnEndPlay()        = 0;
        void SetECSRegister(ECSRegister* _worldRegister);
        void SetSceneManager(SharedPtr<SceneManager>& _sceneManager);
        void SetAppWindowRef(AppWindow* _appWindow);
        void SetCurrentWorld(World* _world);
        void QuitApplication();
        const SharedPtr<SceneManager>& GetSceneManger();

        World* GetWorld();

        Math::Vector2f GetScreenToWorldPosition(Math::Vector2i& _screenPosition);
        Math::Vector2i GetWorldToScreenPosition(Math::Vector2f& _worldPosition);

    protected:
        class World* mCurrentWorld;
        IGameInstance(const IGameInstance&)            = delete; // NO COPY CONSTRUCTOR
        IGameInstance& operator=(const IGameInstance&) = delete; // NO COPY CONSTRUCTOR

#pragma region moveToPC?

        class AppWindow* mAppWindowRef;

    private:
        class ECSRegister* mWorldRegister;
        SharedPtr<SceneManager> mSceneManager;
        void PreInit();
#pragma endregion
    };
} // namespace Umbra
