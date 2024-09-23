#pragma once
#include "EnginePCH.h"
#include "ECS/ECSConfig.h"
#include "ECS/Enity.h"
#include "ECS/Component.h"
#include "ECS/View.h"
#include "ECS/ECSRegister.h"
namespace UMBRA
{
    class System
    {
    private:
        /* data */
    public:
        System(BaseView *view)
        {
            mView = view;
            SystemSignature = view->GetSignature();
        };
        // ~System();
        inline void AddEntity(EntityID _entity) { mView->AddEntity(_entity); };
        inline void RemoveEntity(EntityID _entity) { mView->RemoveEntity(_entity); };
        inline void AssignRegistry(ECSRegister *_register) { mView->AssignRegistry(_register); };
        virtual void Update() = 0;
        ComponentMask SystemSignature;

    protected:
        BaseView *mView;
    };
    
    class SystemManager
    {
    };
}