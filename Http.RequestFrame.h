// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Http.Types.h"
#include "Http.BodyFrame.h"
#include "Basic.SingleByteDecoder.h"
#include "Basic.SingleByteEncoder.h"
#include "Basic.SingleByteEncodingIndex.h"
#include "Http.HeadersFrame.h"

namespace Http
{
    class RequestFrame : public Frame
    {
    private:
        enum State
        {
            receiving_method_state = Start_State,
            receiving_resource_state,
            receiving_protocol_state,
            expecting_LF_after_protocol_state,
            headers_frame_pending_state,
            body_frame_pending_state,
            done_state = Succeeded_State,
            receiving_method_error,
            expecting_LF_after_protocol_error,
            headers_frame_failed,
            body_frame_failed,
        };

        Request* request;
        SingleByteDecoder resource_decoder;
        UnicodeStringRef resource_string;
        BodyFrame body_frame;
        HeadersFrame headers_frame;

        bool ParseError(byte b);

        virtual void IProcess::consider_event(IEvent* event);

    public:
        RequestFrame(Request* request);
    };

    void serialize_request_line(const Request* value, IStream<byte>* stream);

    template <>
    struct __declspec(novtable) serialize<Request>
    {
        void operator()(const Request* value, IStream<byte>* stream) const
        {
            serialize_request_line(value, stream);

            stream->write_elements(Http::globals->CRLF, _countof(Http::globals->CRLF));

            serialize<NameValueCollection>()(value->headers.get(), stream);

            if (value->client_body.get() != 0)
                value->client_body->write_to_stream(stream);
        }
    };
}