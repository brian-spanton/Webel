// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"

namespace Basic
{
    class StateMachine
    {
    private:
        uint32 state;

    protected:
        void switch_to_state(uint32 state);
        void reset();

    public:
        static const uint32 Start_State = 0;
        static const uint32 Succeeded_State = 0x10000;

        StateMachine();

        uint32 get_state();

        virtual bool in_progress();
        virtual bool succeeded();
        virtual bool failed();
    };

    class Frame : public StateMachine, public IProcess
    {
    protected:
        event_result delegate_event_change_state_on_fail(IProcess* process, IEvent* event, uint32 state);

    public:
        virtual bool IProcess::in_progress();
        virtual bool IProcess::succeeded();
        virtual bool IProcess::failed();
    };

    event_result delegate_event(IProcess* process, IEvent* event);
    event_result delegate_event_throw_error_on_fail(IProcess* process, IEvent* event);
    void produce_event(IProcess* process, IEvent* event);
}