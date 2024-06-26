// Copyright � 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Frame.h"
#include "Basic.Globals.h"

namespace Basic
{
    void StateMachine::switch_to_state(uint32 state)
    {
        if (this->state == state)
            throw FatalError("Basic", "StateMachine", "switch_to_state", "this->state == state");

        if (!in_progress())
            throw FatalError("Basic", "StateMachine", "switch_to_state", "!in_progress()");

        this->state = state;

        if (failed())
        {
            char state_string[0x100];
            sprintf_s(state_string, "failed() (state=%X)", this->state);
            Basic::LogDebug("Basic", "StateMachine", "switch_to_state", state_string);
        }
    }

    void StateMachine::reset()
    {
        this->state = Start_State;
    }
        
    void StateMachine::LogUnexpectedEvent(const char* ns, const char* cl, const char* func, IEvent* event)
    {
        char message[0x100];
        sprintf_s(message, "unexpected event %d in state %d", event->get_type(), get_state());
        Basic::LogError(ns, cl, func, message);
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

    ProcessResult Frame::process_event_change_state_on_fail(IProcess* process, IEvent* event, uint32 state)
    {
        ProcessResult result = Basic::process_event(process, event);

        if (process->failed())
        {
            this->switch_to_state(state);
            return ProcessResult::process_result_blocked; // process failed
        }

        return result;
    }

    ProcessResult process_event(IProcess* process, IEvent* event)
    {
        // give up to 0x100000 CPU slices to the current process so long as it reports it is still doing work
        for (uint64 i = 0; i != 0x10000000; i++)
        {
            // doing this check first so that inactive processes never receive events.
            // this simplifies process state machines.
            if (!process->in_progress())
                return ProcessResult::process_result_exited;

            if (process->process_event(event) == process_result_blocked)
                return ProcessResult::process_result_blocked;
        }

        throw FatalError("Basic", "Frame", "process_event", "runaway frame?");
    }

    ProcessResult process_event_throw_error_on_fail(IProcess* process, IEvent* event)
    {
        ProcessResult result = process_event(process, event);

        if (process->failed())
            throw FatalError("Basic", "Frame", "process_event_throw_error_on_fail", "process->failed()");

        return result;
    }

    // an event producer calls this guy, which doesn't care whether the event is fully consumed
    void process_event_ignore_failures(IProcess* process, IEvent* event)
    {
        ProcessResult result = process_event(process, event);
        if (result == ProcessResult::process_result_ready)
            throw FatalError("Basic", "Frame", "process_event_ignore_failures", "result == ProcessResult::process_result_ready");
    }
}