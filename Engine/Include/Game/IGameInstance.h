#pragma once
#include "Core/AppWindow.h"
#include "ECS/ECSRegister.h"
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
        inline void SetECSRegister(ECSRegister* worldRegister) {
            mWorldRegister = worldRegister;
        }
        inline void SetAppWindowRef(AppWindow* appWindow) {
            mAppWindowRef = appWindow;
        }

    protected:
        class ECSRegister* mWorldRegister;

#pragma region moveToPC?
        Math::Vector2f GetScreenToWorldPosition(Math::Vector2i& screenPosition);
        Math::Vector2i GetWorldToScreenPosition(Math::Vector2f& worldPosition);

        class AppWindow* mAppWindowRef;
#pragma endregion
    };
} // namespace Umbra
