#pragma once
#include "EnginePCH.h"
#include "ECS/ECSRegister.h"
namespace UMBRA
{
    class IGameInstance
    {
    public:
        virtual void Initialize() = 0;
        virtual void OnUpdate(float dt) = 0;
        virtual void OnBeginPlay() = 0;
        virtual void OnEndPlay() = 0;
        inline void SetECSRegister(ECSRegister *worldRegister)
        {
            mWorldRegister = worldRegister;
        };
    protected:
        class ECSRegister *mWorldRegister;
    };
}