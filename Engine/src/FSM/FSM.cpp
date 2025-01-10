#include "FSM/FSM.h"

namespace Umbra {

    void FiniteStateMachine::AddState(int _stateId, IFSMState* _state) {
        mStates[_stateId] = _state;
    }

    IFSMState* FiniteStateMachine::GetCurrentState() {
        return mCurrentState;
    }

    void FiniteStateMachine::GoToState(int _stateId) {
        if (mCurrentState == nullptr) {
            mCurrentState = mStates.at(_stateId);
        } else {
            mCurrentState->OnExit();
            mCurrentState = mStates.at(_stateId);
        }
        mCurrentState->OnEnter();
    }
} // namespace Umbra
