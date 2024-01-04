// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Basic.HexNumberStream.h"
#include "Http.LengthBodyFrame.h"
#include "Http.HeadersFrame.h"

namespace Http
{
    using namespace Basic;

    // RFC2616 3.6.1
    class BodyChunksFrame : public BodyFrame
    {
    private:
        enum State
        {
            start_chunk_state = Start_State,
            expecting_LF_after_size_state,
            chunk_frame_pending_state,
            expecting_CR_after_chunk_state,
            expecting_LF_after_chunk_state,
            headers_frame_pending,
            done_state = Succeeded_State,
            start_chunk_error,
            expecting_LF_after_size_error,
            chunk_frame_failed,
            expecting_CR_after_chunk_error,
            expecting_LF_after_chunk_error,
            header_frame_failed,
        };

        uint32 size;
        HexNumberStream<byte, uint32> size_stream;
        LengthBodyFrame chunk_frame;
        HeadersFrame headers_frame;

        ProcessResult IProcess::process_event(IEvent* event);

    public:
        BodyChunksFrame(std::shared_ptr<IStream<byte> > body_stream, NameValueCollection* headers);
    };
}