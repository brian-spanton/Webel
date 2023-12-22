// Copyright © 2013 Brian Spanton

#pragma once

namespace Basic
{
    __interface IEvent;

    enum EventResult : bool
    {
        event_result_yield = false,
        event_result_continue = true,
    };

    __interface IProcess
    {
        EventResult consider_event(IEvent* event);
        bool in_progress();
        bool succeeded();
        bool failed();
    };
}