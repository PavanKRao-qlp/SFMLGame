#pragma once
#include "Core/AppWindow.h"
#include "Game/SceneManager.h"
#include "UI/ImGuiBackend.h"
#include "Umbra.h"

#include "SFML/Graphics.hpp"
namespace Umbra {
    class IGameInstance {
    public:
        IGameInstance()  = default;
        ~IGameInstance() = default;
        virtual FGameConfig& LoadGameConfig();
        virtual void Initialize() = 0;
        virtual void ShutDown()   = 0;
        void QuitApplication();
        SceneManager& GetSceneManger();
        ImGuiBackend& GetUIManager();

        // virtual void OnUpdate(float dt) = 0;
        // virtual void OnBeginPlay()      = 0;
        // virtual void OnEndPlay()        = 0;
        // void SetECSRegister(ECSRegister* _worldRegister);
        // void SetSceneManager(SharedPtr<SceneManager>& _sceneManager);
        // void SetAppWindowRef(AppWindow* _appWindow);
        // void SetCurrentWorld(World* _world);
        // World* GetWorld();
        // Math::Vector2f GetScreenToWorldPosition(Math::Vector2i& _screenPosition);
        // Math::Vector2i GetWorldToScreenPosition(Math::Vector2f& _worldPosition);
        // class AppWindow* mAppWindowRef;
    protected:
        class World* mCurrentWorld;
        IGameInstance(const IGameInstance&)            = delete; // NO COPY CONSTRUCTOR
        IGameInstance& operator=(const IGameInstance&) = delete; // NO COPY CONSTRUCTOR
    private:
        friend class App;
        SceneManager* mSceneManager;
        ImGuiBackend* mUIBackend;
    };
} // namespace Umbra
