#pragma once
#include "Core/AppWindow.h"
#include "Core/Clock.h"
#include "ECS/ECSRegister.h"
#include "Game/World.h"
#include "Math/Vector.h"
#include "Umbra.h"

#include "SFML/Graphics.hpp"
namespace Umbra {
    class IGameInstance {
    public:
        virtual void Initialize()       = 0;
        virtual void OnUpdate(float dt) = 0;
        virtual void OnBeginPlay()      = 0;
        virtual void OnEndPlay()        = 0;
        void SetECSRegister(ECSRegister* worldRegister);
        void SetAppWindowRef(AppWindow* appWindow);
        void SetCurrentWorld(World* _world);

        World* GetWorld();

        Math::Vector2f GetScreenToWorldPosition(Math::Vector2i& screenPosition);
        Math::Vector2i GetWorldToScreenPosition(Math::Vector2f& worldPosition);

    protected:
        class World* mCurrentWorld;

#pragma region moveToPC?

        class AppWindow* mAppWindowRef;

    private:
        class ECSRegister* mWorldRegister;
        void PreInit();
#pragma endregion
    };
} // namespace Umbra
