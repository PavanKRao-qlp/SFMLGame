#include "Game/IGameInstance.h"
namespace Umbra {

    void IGameInstance::SetECSRegister(ECSRegister* worldRegister) {
        mWorldRegister = worldRegister;
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
} // namespace Umbra
