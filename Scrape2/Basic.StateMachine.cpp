// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.StateMachine.h"

namespace Basic
{
    void StateMachine::switch_to_state(uint32 state)
    {
        if (this->state == state)
            throw FatalError("unexpected switch to current state");

        if (!in_progress())
            throw FatalError("unexpected switch from completed state");

        this->state = state;

        if (failed())
        {
            char state_string[0x40];
            sprintf_s(state_string, "state=%X", this->state);
            HandleError(state_string);
        }
    }

    void StateMachine::reset()
    {
        this->state = Start_State;
    }

    StateMachine::StateMachine() :
        state(Start_State)
    {
    }

    uint32 StateMachine::get_state()
    {
        return this->state;
    }

    bool StateMachine::in_progress()
    {
        return (this->state < Succeeded_State);
    }

    bool StateMachine::succeeded()
    {
        return (this->state == Succeeded_State);
    }

    bool StateMachine::failed()
    {
        return (this->state > Succeeded_State);
    }
}