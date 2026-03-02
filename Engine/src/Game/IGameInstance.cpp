#include "Game/IGameInstance.h"

namespace Umbra {

    FGameConfig IGameInstance::LoadGameConfig() {
        return FGameConfig();
    }

    void IGameInstance::QuitApplication() {
        EventBus::FireEvent<AppClosedEvent>(AppClosedEvent());
    }

    SceneManager& IGameInstance::GetSceneManager() {
        return *mSceneManager;
    }
    ImGuiBackend& Umbra::IGameInstance::GetUIManager() {
        return *mUIBackend;
    }


} // namespace Umbra
