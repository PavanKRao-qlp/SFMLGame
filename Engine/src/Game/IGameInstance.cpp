#include "Game/IGameInstance.h"

namespace Umbra {

    FGameConfig& IGameInstance::LoadGameConfig() {
        return FGameConfig();
    }

    void IGameInstance::QuitApplication() {
        AppClosedEvent* event = new AppClosedEvent();
        EventBus::FireEvent<AppClosedEvent>(event);
    }

    SceneManager& IGameInstance::GetSceneManger() {
        return *mSceneManager;
    }

} // namespace Umbra
