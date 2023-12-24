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

    EventResult DisconnectBodyFrame::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::receiving_body_state:
            {
                if (event->get_type() == Basic::EventType::element_stream_ending_event)
                {
                    switch_to_state(State::done_state);
                    return EventResult::event_result_continue;
                }

                const byte* elements;
                uint32 count;

                EventResult result = Event::Read(event, 0xffffffff, &elements, &count);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                this->decoded_content_stream->write_elements(elements, count);
            }
            break;

        default:
            throw FatalError("Http::DisconnectBodyFrame::handle_event unexpected state");
        }

        return EventResult::event_result_continue;
    }
}