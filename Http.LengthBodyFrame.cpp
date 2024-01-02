// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Http.LengthBodyFrame.h"
#include "Basic.Event.h"

namespace Http
{
    using namespace Basic;

    LengthBodyFrame::LengthBodyFrame(std::shared_ptr<IStream<byte> > body_stream) :
        BodyFrame(body_stream),
        bytes_expected(0),
        bytes_received(0)
    {
    }

    LengthBodyFrame::LengthBodyFrame(std::shared_ptr<IStream<byte> > body_stream, uint32 bytes_expected) :
        BodyFrame(body_stream)
    {
        reset(bytes_expected);
    }

    void LengthBodyFrame::reset(uint32 bytes_expected)
    {
        __super::reset();

        this->bytes_expected = bytes_expected;
        this->bytes_received = 0;

        if (this->bytes_expected == 0)
        {
            this->decoded_content_stream->write_eof();
            switch_to_state(State::done_state);
        }
    }

    ProcessResult LengthBodyFrame::process_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::receiving_body_state:
            {
                const byte* elements;
                uint32 useable;

                ProcessResult result = Event::Read(event, this->bytes_expected - this->bytes_received, &elements, &useable);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                this->decoded_content_stream->write_elements(elements, useable);

                this->bytes_received += useable;

                if (this->bytes_received == this->bytes_expected)
                {
                    this->decoded_content_stream->write_eof();
                    switch_to_state(State::done_state);
                    return ProcessResult::process_result_ready;
                }
            }
            break;

        default:
            throw FatalError("Http::LengthBodyFrame::handle_event unexpected state");
        }

        return ProcessResult::process_result_ready;
    }
}