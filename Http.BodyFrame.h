// Copyright © 2013 Brian Spanton

#pragma once

#include "Http.LengthBodyFrame.h"
#include "Http.BodyChunksFrame.h"
#include "Http.DisconnectBodyFrame.h"
#include "Http.HeadersFrame.h"
#include "Basic.IStream.h"

namespace Http
{
    using namespace Basic;

    class BodyFrame : public Frame
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

        NameValueCollection::Ref headers; // REF
        Ref<IStream<byte> > body_stream; // REF
        Inline<BodyChunksFrame> chunks_frame;
        Inline<LengthBodyFrame> chunk_frame;
        Inline<DisconnectBodyFrame> disconnect_frame;
        Inline<HeadersFrame> headers_frame;

        void switch_to_state(State state);

    public:
        void Initialize(NameValueCollection* headers);

        void set_body_stream(IStream<byte>* body_stream);
        virtual void IProcess::Process(IEvent* event, bool* yield);
    };
}