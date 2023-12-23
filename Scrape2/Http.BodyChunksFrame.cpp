// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Http.Globals.h"
#include "Http.BodyChunksFrame.h"
#include "Http.Types.h"

namespace Http
{
    using namespace Basic;

    BodyChunksFrame::BodyChunksFrame(std::shared_ptr<IStream<byte> > body_stream) :
        size(0),
        size_stream(&this->size), // initialization is in order of declaration in class def
        chunk_frame(body_stream)
    {
    }

    bool BodyChunksFrame::write_elements(IElementSource<byte>* element_source)
    {
        switch (get_state())
        {
		case State::start_chunk_state:
        case State::expecting_LF_after_size_state:
        case State::expecting_CR_after_chunk_state:
        case State::expecting_LF_after_chunk_state:
            {
				while (true)
				{
					byte b;

					bool success = element_source->ReadNext(&b);
					if (!success)
						return false;

					write_element(b);
					// $$$$

				}
            }
            break;

        case State::chunk_frame_pending_state:
            {
                bool done = this->chunk_frame.write_elements(element_source);

                if (!done)
                    return;

                switch_to_state(State::expecting_CR_after_chunk_state);
            }
            break;

        default:
            throw FatalError("BodyChunksFrame::handle_event unexpected state");
        }
    }

    void BodyChunksFrame::write_element(byte b)
    {
        switch (get_state())
        {
        case State::start_chunk_state:
            {
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
            {
                bool done = this->chunk_frame.write_elements(&b, 1);

                if (!done)
                    return;

                switch_to_state(State::expecting_CR_after_chunk_state);
            }
            break;

        case State::expecting_CR_after_chunk_state:
            {
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