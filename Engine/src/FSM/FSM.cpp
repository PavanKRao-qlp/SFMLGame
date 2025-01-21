#include "FSM/FSM.h"

namespace Umbra {

    void FiniteStateMachine::AddState(int _stateId, IFSMState* _state) {
        mStates[_stateId] = _state;
        _state->SetFSMRef(this);
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

    void IFSMState::SetFSMRef(FiniteStateMachine* _FSM) {
        mFSM = _FSM;
    }

} // namespace Umbra
