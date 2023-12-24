// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Http.Globals.h"
#include "Http.BodyChunksFrame.h"
#include "Http.Types.h"
#include "Basic.Event.h"

namespace Http
{
    using namespace Basic;

    BodyChunksFrame::BodyChunksFrame(std::shared_ptr<IStream<byte> > body_stream, std::shared_ptr<NameValueCollection> headers) :
        BodyFrame(body_stream),
        size(0),
        size_stream(&this->size), // initialization is in order of declaration in class def
        chunk_frame(body_stream),
        headers_frame(headers.get())
    {
    }

    EventResult BodyChunksFrame::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::start_chunk_state:
            {
                byte b;
                EventResult result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                if (b == Http::globals->CR)
                {
                    switch_to_state(State::expecting_LF_after_size_state);
                }
                else
                {
                    bool success = this->size_stream.WriteDigit(b);
                    if (!success)
                    {
                        switch_to_state(State::start_chunk_error);
                        return EventResult::event_result_continue;
                    }
                }
            }
            break;

        case State::expecting_LF_after_size_state:
            {
                byte b;
                EventResult result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                if (b != Http::globals->LF)
                {
                    switch_to_state(State::expecting_LF_after_size_error);
                    return EventResult::event_result_continue;
                }

                if (this->size == 0)
                {
                    // $ I don't remember why this is getting headers at the end after a chunked body, check the RFC.
                    // however, not doing this definitely causes problems due to the check in ConnectedSocket::Receive:
                    // "if (!this->protocol_element_source.Exhausted() && !protocol->failed())"

                    switch_to_state(State::headers_frame_pending);
                }
                else
                {
                    this->chunk_frame.reset(this->size);
                    switch_to_state(State::chunk_frame_pending_state);
                }
            }
            break;

        case State::chunk_frame_pending_state:
            {
                EventResult result = delegate_event_change_state_on_fail(&this->chunk_frame, event, State::chunk_frame_failed);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                switch_to_state(State::expecting_CR_after_chunk_state);
            }
            break;

        case State::expecting_CR_after_chunk_state:
            {
                byte b;
                EventResult result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                if (b != Http::globals->CR)
                {
                    switch_to_state(State::expecting_CR_after_chunk_error);
                    return EventResult::event_result_continue;
                }

                switch_to_state(State::expecting_LF_after_chunk_state);
            }
            break;

        case State::expecting_LF_after_chunk_state:
            {
                byte b;
                EventResult result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                if (b != Http::globals->LF)
                {
                    switch_to_state(State::expecting_LF_after_chunk_error);
                    return EventResult::event_result_continue;
                }

                this->size_stream.reset();
                switch_to_state(State::start_chunk_state);
            }
            break;

        case State::headers_frame_pending:
            {
                EventResult result = delegate_event_change_state_on_fail(&this->headers_frame, event, State::header_frame_failed);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                switch_to_state(State::done_state);
            }
            break;

        default:
            throw FatalError("BodyChunksFrame::handle_event unexpected state");
        }

        return EventResult::event_result_continue;
    }
}