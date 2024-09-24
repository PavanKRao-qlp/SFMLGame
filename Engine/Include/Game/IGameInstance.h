#pragma once
#include "ECS/ECSRegister.h"
#include "EnginePCH.h"
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

    protected:
        class ECSRegister* mWorldRegister;
    };
} // namespace Umbra
