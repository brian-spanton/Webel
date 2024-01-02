// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Basic.DecNumberStream.h"
#include "Http.Types.h"
#include "Http.HeadersFrame.h"
#include "Http.BodyFrame.h"
#include "Basic.SingleByteEncoder.h"
#include "Basic.SingleByteEncodingIndex.h"

namespace Http
{
    using namespace Basic;

    // ResponseFrame is factored out of Web.Client for readability and ease
    // of "resetting" for a new get request from the same client.
    // It is not combined with ResponseBodyFrSame so the expense of that set of state
    // is not incurred when processing responses with no body.
    class ResponseFrame : public Frame
    {
    private:
        enum State
        {
            receiving_protocol_state = Start_State,
            receiving_code_state,
            receiving_reason_state,
            expecting_LF_after_reason_state,
            headers_frame_pending_state,
            body_pending_state,
            done_state = Succeeded_State,
            receiving_code_error,
            write_to_number_stream_failed,
            receiving_reason_error,
            expecting_LF_after_reason_error,
            headers_frame_failed,
            body_failed,
        };

        std::shared_ptr<Transaction> transaction;
        DecNumberStream<byte, uint16> number_stream;
        HeadersFrame headers_frame;
        std::shared_ptr<IStream<byte> > decoded_content_stream;
        std::shared_ptr<BodyFrame> body_frame;
        std::weak_ptr<IProcess> completion;
        ByteStringRef completion_cookie;

        virtual ProcessResult IProcess::consider_event(IEvent* event);

    public:
        ResponseFrame(std::shared_ptr<Transaction> transaction, std::shared_ptr<IProcess> completion, ByteStringRef cookie);

        void set_decoded_content_stream(std::shared_ptr<IStream<byte> > decoded_content_stream);
    };

    template <>
    struct __declspec(novtable) serialize<Response>
    {
        void operator()(const Response* value, IStream<byte>* stream) const
        {
            SingleByteEncoder encoder;
            encoder.Initialize(Basic::globals->ascii_index, stream);

            value->protocol->write_to_stream(&encoder);

            UnicodeString code;
            TextWriter writer(&code);
            writer.WriteFormat<0x10>("%d", value->code);

            stream->write_element(Http::globals->SP);
            code.write_to_stream(&encoder);

            stream->write_element(Http::globals->SP);
            value->reason->write_to_stream(&encoder);

            stream->write_elements(Http::globals->CRLF, _countof(Http::globals->CRLF));
            serialize<NameValueCollection>()(value->headers.get(), stream);

            if (value->response_body.get() != 0)
                value->response_body->write_to_stream(stream);
        }
    };
}