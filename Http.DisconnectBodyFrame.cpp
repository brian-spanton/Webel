// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Http.DisconnectBodyFrame.h"
#include "Basic.Event.h"

namespace Http
{
    using namespace Basic;

    DisconnectBodyFrame::DisconnectBodyFrame(std::shared_ptr<IStream<byte> > body_stream) :
        body_stream(body_stream)
    {
    }

    void DisconnectBodyFrame::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::receiving_body_state:
            {
                if (event->get_type() == EventType::element_stream_ending_event)
                {
                    switch_to_state(State::done_state);
                    return;
                }

                const byte* elements;
                uint32 count;

                Event::Read(event, 0xffffffff, &elements, &count);

                this->body_stream->write_elements(elements, count);
            }
            break;

        default:
            throw FatalError("Http::DisconnectBodyFrame::handle_event unexpected state");
        }
    }
}