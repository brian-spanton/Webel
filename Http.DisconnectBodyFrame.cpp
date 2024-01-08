// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Http.DisconnectBodyFrame.h"
#include "Basic.Event.h"

namespace Http
{
    using namespace Basic;

    DisconnectBodyFrame::DisconnectBodyFrame(std::shared_ptr<IStream<byte> > body_stream) :
        BodyFrame(body_stream)
    {
    }

    ProcessResult DisconnectBodyFrame::process_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::receiving_body_state:
            {
                if (event->get_type() == Basic::EventType::element_stream_ending_event)
                {
                    this->decoded_content_stream->write_eof();
                    switch_to_state(State::done_state);
                    return ProcessResult::process_result_ready;
                }

                const byte* elements;
                uint32 count;

                ProcessResult result = Event::Read(event, 0xffffffff, &elements, &count);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                this->decoded_content_stream->write_elements(elements, count);
            }
            break;

        default:
            throw FatalError("Http", "DisconnectBodyFrame::process_event unhandled state");
        }

        return ProcessResult::process_result_ready;
    }
}