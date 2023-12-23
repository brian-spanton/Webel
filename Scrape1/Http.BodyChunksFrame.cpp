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
        size_stream(&this->size), // initialization is in order of declaration in class def
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
                    {
                        switch_to_state(State::start_chunk_error);
                        return;
                    }
                }
            }
            break;

        case State::expecting_LF_after_size_state:
            {
                byte b;
                Event::ReadNext(event, &b);

                if (b != Http::globals->LF)
                {
                    switch_to_state(State::expecting_LF_after_size_error);
                    return;
                }

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
            break;

        case State::chunk_frame_pending_state:
            delegate_event_change_state_on_fail(&this->chunk_frame, event, State::chunk_frame_failed);
            switch_to_state(State::expecting_CR_after_chunk_state);
            break;

        case State::expecting_CR_after_chunk_state:
            {
                byte b;
                Event::ReadNext(event, &b);

                if (b != Http::globals->CR)
                {
                    switch_to_state(State::expecting_CR_after_chunk_error);
                    return;
                }

                switch_to_state(State::expecting_LF_after_chunk_state);
            }
            break;

        case State::expecting_LF_after_chunk_state:
            {
                byte b;
                Event::ReadNext(event, &b);

                if (b != Http::globals->LF)
                {
                    switch_to_state(State::expecting_LF_after_chunk_error);
                    return;
                }

                this->size_stream.reset();
                switch_to_state(State::start_chunk_state);
            }
            break;

        default:
            throw FatalError("BodyChunksFrame::handle_event unexpected state");
        }
    }
}