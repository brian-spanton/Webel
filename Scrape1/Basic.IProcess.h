// Copyright © 2013 Brian Spanton

#pragma once

namespace Basic
{
    __interface IEvent;

    __interface IProcess
    {
        void consider_event(IEvent* event);
        bool in_progress();
        bool succeeded();
        bool failed();
    };
}