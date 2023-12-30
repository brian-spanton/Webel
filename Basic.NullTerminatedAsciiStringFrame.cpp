// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.NullTerminatedAsciiStringFrame.h"
#include "Basic.Globals.h"
#include "Basic.Event.h"

namespace Basic
{
    NullTerminatedAsciiStringFrame::NullTerminatedAsciiStringFrame(Basic::IStream<Codepoint>* destination) :
        decoder(Basic::globals->ascii_index, destination)
    {
    }

    EventResult NullTerminatedAsciiStringFrame::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::receiving_state:
            {
                byte b;
                EventResult result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                if (b == 0)
                {
                    decoder.write_eof();
                    switch_to_state(State::done_state);
                    break;
                }

                decoder.write_element(b);
            }
            break;

        default:
            throw FatalError("Basic::StreamFrame::handle_event unexpected state");
        }

        return EventResult::event_result_continue;
    }
}