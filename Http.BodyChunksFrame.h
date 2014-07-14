// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Http.LengthBodyFrame.h"
#include "Basic.HexNumberStream.h"

namespace Http
{
    using namespace Basic;

    // RFC 2616 3.6.1
    class BodyChunksFrame : public Frame
    {
    private:
        enum State
        {
            start_chunk_state = Start_State,
            expecting_LF_after_size_state,
            chunk_frame_pending_state,
            expecting_LF_after_chunk_state,
            done_state = Succeeded_State,
            start_chunk_error,
            expecting_LF_after_size_error,
            chunk_frame_failed,
            expecting_CR_after_chunk_error,
            expecting_LF_after_chunk_error,
        };

        HexNumberStream<byte, uint32> size_stream;
        uint32 size;
        LengthBodyFrame chunk_frame;

        virtual void IProcess::consider_event(IEvent* event);

    public:
        BodyChunksFrame(std::shared_ptr<IStream<byte> > body_stream);
    };
}