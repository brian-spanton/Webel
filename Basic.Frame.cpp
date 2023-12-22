// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Frame.h"

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

    bool Frame::in_progress()
    {
        return StateMachine::in_progress();
    }

    bool Frame::succeeded()
    {
        return StateMachine::succeeded();
    }

    bool Frame::failed()
    {
        return StateMachine::failed();
    }

    EventResult Frame::delegate_event_change_state_on_fail(IProcess* process, IEvent* event, uint32 state)
    {
        EventResult result = delegate_event(process, event);

        if (process->failed())
        {
            this->switch_to_state(state);
            return event_result_yield; // process failed
        }

        return result;
    }

    EventResult delegate_event(IProcess* process, IEvent* event)
    {
        // give up to 0x100000 CPU slices to the current process so long as it reports it is still doing work
        for (uint64 i = 0; i != 0x100000; i++)
        {
            // doing this check first ensures that processes that have already failed or succeeded don't have to
            // handle further events...
            // $$ could this hide some transgressions though?
            if (!process->in_progress())
                return event_result_continue;

            if (process->consider_event(event) == event_result_yield)
                return event_result_yield;
        }

        throw FatalError("runaway frame?");
    }

    EventResult delegate_event_throw_error_on_fail(IProcess* process, IEvent* event)
    {
        EventResult result = delegate_event(process, event);

        if (process->failed())
            throw FatalError("delegate_event_throw_error_on_fail process->failed()");

        return result;
    }

    void produce_event(IProcess* process, IEvent* event)
    {
        delegate_event(process, event);
    }
}