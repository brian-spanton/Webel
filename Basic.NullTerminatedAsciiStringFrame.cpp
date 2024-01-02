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

    ProcessResult NullTerminatedAsciiStringFrame::process_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::receiving_state:
            {
                byte b;
                ProcessResult result = Event::ReadNext(event, &b);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

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

        return ProcessResult::process_result_ready;
    }
}