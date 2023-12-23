// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.StateMachine.h"
#include "Http.LengthBodyFrame.h"
#include "Basic.HexNumberStream.h"
#include "Basic.IStream.h"

namespace Http
{
    using namespace Basic;

    // RFC 2616 3.6.1
    class BodyChunksFrame : public StateMachine
    {
    private:
        enum State
        {
            start_chunk_state = Start_State,
            expecting_LF_after_size_state,
            chunk_frame_pending_state,
            expecting_CR_after_chunk_state,
            expecting_LF_after_chunk_state,
            done_state = Succeeded_State,
            start_chunk_error,
            expecting_LF_after_size_error,
            expecting_CR_after_chunk_error,
            expecting_LF_after_chunk_error,
        };

        uint32 size;
        HexNumberStream<byte, uint32> size_stream;
        LengthBodyFrame chunk_frame;

        void write_element(byte element);

    public:
        BodyChunksFrame(std::shared_ptr<IStream<byte> > body_stream);

        bool write_elements(IElementSource<byte>* element_source);
    };
}