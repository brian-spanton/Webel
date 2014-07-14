// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Http.Globals.h"
#include "Http.BodyChunksFrame.h"
#include "Http.Types.h"
#include "Basic.Event.h"

namespace Http
{
    using namespace Basic;

    BodyChunksFrame::BodyChunksFrame(std::shared_ptr<IStream<byte> > body_stream) :
        size(0),
        size_stream(&this->size),
        chunk_frame(body_stream)
    {
    }

    void BodyChunksFrame::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::start_chunk_state:
            {
                byte b;
                Event::ReadNext(event, &b);

                if (b == Http::globals->CR)
                {
                    switch_to_state(State::expecting_LF_after_size_state);
                }
                else
                {
                    bool success = this->size_stream.WriteDigit(b);
                    if (!success)
                        switch_to_state(State::start_chunk_error);
                }
            }
            break;

        case State::expecting_LF_after_size_state:
            {
                byte b;
                Event::ReadNext(event, &b);

                if (b == Http::globals->LF)
                {
                    if (this->size == 0)
                    {
                        switch_to_state(State::done_state);
                    }
                    else
                    {
                        this->chunk_frame.reset(this->size);
                        switch_to_state(State::chunk_frame_pending_state);
                    }
                }
                else
                {
                    switch_to_state(State::expecting_LF_after_size_error);
                }
            }
            break;

        case State::chunk_frame_pending_state:
            {
                delegate_event_change_state_on_fail(&this->chunk_frame, event, State::chunk_frame_failed);

                byte b;
                Event::ReadNext(event, &b);

                if (b == Http::globals->CR)
                {
                    switch_to_state(State::expecting_LF_after_chunk_state);
                }
                else
                {
                    switch_to_state(State::expecting_CR_after_chunk_error);
                }
            }
            break;

        case State::expecting_LF_after_chunk_state:
            {
                byte b;
                Event::ReadNext(event, &b);

                if (b == Http::globals->LF)
                {
                    this->size_stream.reset();
                    switch_to_state(State::start_chunk_state);
                }
                else
                {
                    switch_to_state(State::expecting_LF_after_chunk_error);
                }
            }
            break;

        default:
            throw FatalError("BodyChunksFrame::handle_event unexpected state");
        }
    }
}