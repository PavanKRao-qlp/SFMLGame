#pragma once
#include "ECS/Component.h"
#include "ECS/ECSConfig.h"
#include "ECS/ECSRegister.h"
#include "ECS/Enity.h"
#include "ECS/View.h"
#include "EnginePCH.h"
namespace Umbra {
    enum class ESystemPhase {
        FrameStart, // Input collection, etc.
        Simulation, // Physics, AI
        PreRender, // Visibility culling
        Render, // Actual drawing
        FrameEnd // Cleanup
    };

    class System {
    private:
        /* data */
    public:
        System(BaseView* view) {
            mView           = view;
            SystemSignature = view->GetSignature();
        }
        // ~System();
        inline virtual void AddEntity(EntityID _entity) {
            mView->AddEntity(_entity);
        }
        inline void Flush() {
            mView->Flush();
        }
        inline virtual void RemoveEntity(EntityID _entity) {
            mView->RemoveEntity(_entity);
        }
        inline void AssignRegistry(ECSRegister* _register) {
            mView->AssignRegistry(_register);
        }
        inline void SetEnabled(bool bShouldEnable) {
            bEnabled = bShouldEnable;
        }
        inline bool GetEnabled() {
            return bEnabled;
        }
        virtual void Update() = 0;
        ComponentMask SystemSignature;

    protected:
        BaseView* mView;
        bool bEnabled       = true;
        ESystemPhase mPhase = ESystemPhase::Simulation;
        int mPriority       = 0;
    };

    class SystemManager {};
} // namespace Umbra
