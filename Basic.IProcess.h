// Copyright © 2013 Brian Spanton

#pragma once

namespace Basic
{
    __interface IEvent;

    enum EventResult
    {
        event_result_yield = 0,
        event_result_continue,
        event_result_process_inactive,
    };

    __interface IProcess
    {
        EventResult consider_event(IEvent* event);
        bool in_progress();
        bool succeeded();
        bool failed();
    };
}