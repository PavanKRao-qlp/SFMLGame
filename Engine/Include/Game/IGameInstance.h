#pragma once
#include "Core/AppWindow.h"
#include "Game/SceneManager.h"
#include "UI/ImGuiBackend.h"
#include "Umbra.h"
namespace Umbra {
    class IGameInstance {
    public:
        IGameInstance()  = default;
        ~IGameInstance() = default;
        virtual FGameConfig LoadGameConfig();
        virtual void Initialize() = 0;
        virtual void ShutDown()   = 0;
        void QuitApplication();
        SceneManager& GetSceneManager();
        ImGuiBackend& GetUIManager();

        // Set to true to enable the debug overlay (toggle with Alt + ~)
        bool bDebug = true;

    protected:
        IGameInstance(const IGameInstance&)            = delete; // NO COPY CONSTRUCTOR
        IGameInstance& operator=(const IGameInstance&) = delete; // NO COPY CONSTRUCTOR
    private:
        friend class App;
        SceneManager* mSceneManager;
        ImGuiBackend* mUIBackend;
    };
} // namespace Umbra
