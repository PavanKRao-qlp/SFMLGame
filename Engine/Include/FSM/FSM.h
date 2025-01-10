#pragma once
#include "Umbra.h"
namespace Umbra {
    class IFSMState {
    public:
        virtual void OnEnter()  = 0;
        virtual void OnUpdate() = 0;
        virtual void OnExit()   = 0;
    };

    class FiniteStateMachine {

    public:
        void AddState(int _stateId, IFSMState* _state);
        IFSMState* GetCurrentState();
        void GoToState(int _stateId);

    private:
        UMap<int, IFSMState*> mStates;
        IFSMState* mCurrentState;
    };

} // namespace Umbra
