#include "Game/IGameInstance.h"

#include "Core/AppWindow.h"

namespace Umbra {

    FGameConfig& IGameInstance::LoadGameConfig() {
        return FGameConfig();
    }

    void IGameInstance::SetECSRegister(ECSRegister* worldRegister) {
        mWorldRegister = worldRegister;
    }

    void IGameInstance::SetSceneManager(SharedPtr<SceneManager>& _sceneManager) {
        mSceneManager = _sceneManager;
    }


    void IGameInstance::SetAppWindowRef(AppWindow* appWindow) {
        mAppWindowRef = appWindow;
    }
    void IGameInstance::SetCurrentWorld(World* _world) {
        mCurrentWorld = _world;
    }
    World* IGameInstance::GetWorld() {
        return mCurrentWorld;
    }
    Math::Vector2f IGameInstance::GetScreenToWorldPosition(Math::Vector2i& screenPosition) {

        sf::View view           = mAppWindowRef->GetRenderWindowHandle()->getView();
        sf::Vector2f sfWorldPos = mAppWindowRef->GetRenderWindowHandle()->mapPixelToCoords(
            sf::Vector2i(screenPosition.x, screenPosition.y), view);
        return Math::Vector2f(sfWorldPos.x, sfWorldPos.y);
    }

    Math::Vector2i IGameInstance::GetWorldToScreenPosition(Math::Vector2f& worldPosition) {
        sf::View view            = mAppWindowRef->GetRenderWindowHandle()->getView();
        sf::Vector2i sfScreenPos = mAppWindowRef->GetRenderWindowHandle()->mapCoordsToPixel(
            sf::Vector2f(worldPosition.x, worldPosition.y), view);
        return Math::Vector2i(sfScreenPos.x, sfScreenPos.y);
    }

    void Umbra::IGameInstance::QuitApplication() {
        AppClosedEvent* event = new AppClosedEvent();
        EventBus::FireEvent<AppClosedEvent>(event);
    }
    const SharedPtr<SceneManager>& IGameInstance::GetSceneManger() {
        return mSceneManager;
    }

} // namespace Umbra
