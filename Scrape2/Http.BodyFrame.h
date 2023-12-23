// Copyright © 2013 Brian Spanton

#pragma once

#include "Http.LengthBodyFrame.h"
#include "Http.BodyChunksFrame.h"
#include "Http.HeadersFrame.h"
#include "Basic.IStream.h"

namespace Http
{
    using namespace Basic;

    class BodyFrame : public StateMachine, public IElementConsumer<byte>
    {
    private:
        enum State
        {
            start_state = Start_State,
            chunks_frame_pending_state,
            chunk_frame_pending_state,
            disconnect_frame_pending_state,
            headers_frame_pending,
            done_state = Succeeded_State,
            unhandled_content_encoding_error,
            unhandled_transfer_encoding_error,
            no_content_type_error,
            chunks_frame_failed,
            chunk_frame_failed,
            disconnect_frame_failed,
            header_frame_failed,
        };

        std::shared_ptr<NameValueCollection> headers;
        std::shared_ptr<IStream<byte> > body_stream;
        std::shared_ptr<BodyChunksFrame> chunks_frame;
        std::shared_ptr<LengthBodyFrame> chunk_frame;
        HeadersFrame headers_frame;

        void switch_to_state(State state);

    public:
        BodyFrame(std::shared_ptr<NameValueCollection> headers);

        void set_body_stream(std::shared_ptr<IStream<byte> > body_stream);

		ConsumeElementsResult IElementConsumer<byte>::consume_elements(IElementSource<byte>* element_source);
	};
}