// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Http.RequestFrame.h"
#include "Http.Globals.h"
#include "Basic.Event.h"
#include "Basic.CountStream.h"
#include "Basic.Globals.h"

namespace Http
{
    using namespace Basic;

    RequestFrame::RequestFrame(Request* request) :
        request(request),
        resource_string(std::make_shared<UnicodeString>()),
        resource_decoder(Basic::globals->ascii_index, this->resource_string.get()),
        headers_frame(this->request->headers.get())
    {
        this->resource_string->reserve(0x100);
    }

    ProcessResult RequestFrame::process_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::receiving_method_state:
            {
                byte b;
                ProcessResult result = Event::ReadNext(event, &b);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                if (b == Http::globals->SP)
                {
                    switch_to_state(State::receiving_resource_state);
                }
                else if (Http::globals->TOKEN[b])
                {
                    this->request->method->push_back(b);
                }
                else
                {
                    switch_to_state(State::receiving_method_error);
                }
            }
            break;

        case State::receiving_resource_state:
            {
                byte b;
                ProcessResult result = Event::ReadNext(event, &b);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                if (b == Http::globals->SP)
                {
                    this->resource_decoder.write_eof();

                    Uri base;
                    base.scheme = Basic::globals->http_scheme;

                    this->request->resource->Parse(this->resource_string.get(), &base);
                    switch_to_state(State::receiving_protocol_state);
                }
                else
                {
                    this->resource_decoder.write_element(b);
                }
            }
            break;

        case State::receiving_protocol_state:
            {
                byte b;
                ProcessResult result = Event::ReadNext(event, &b);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                if (b == Http::globals->CR)
                {
                    switch_to_state(State::expecting_LF_after_protocol_state);
                }
                else
                {
                    this->request->protocol->push_back(b);
                }
            }
            break;

        case State::expecting_LF_after_protocol_state:
            {
                byte b;
                ProcessResult result = Event::ReadNext(event, &b);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                if (b == Http::globals->LF)
                {
                    switch_to_state(State::headers_frame_pending_state);
                }
                else
                {
                    switch_to_state(State::expecting_LF_after_protocol_error);
                }
            }
            break;

        case State::headers_frame_pending_state:
            {
                ProcessResult result = process_event_change_state_on_fail(&this->headers_frame, event, State::headers_frame_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                std::shared_ptr<CountStream<byte> > count_stream = std::make_shared<CountStream<byte> >();
                std::shared_ptr<Transaction> transaction = std::make_shared<Transaction>();

                transaction->request = this->request;

                BodyFrame::make_body_frame(count_stream, transaction.get(), &this->body_frame);
                if (!this->body_frame)
                {
                    switch_to_state(State::done_state);
                    return ProcessResult::process_result_ready;
                }

                switch_to_state(State::body_frame_pending_state);
            }
            break;

        case State::body_frame_pending_state:
            {
                ProcessResult result = process_event_change_state_on_fail(this->body_frame.get(), event, State::body_frame_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                switch_to_state(State::done_state);
            }
            break;

        default:
            throw FatalError("Http::RequestFrame::handle_event unexpected state");
        }

        return ProcessResult::process_result_ready;
    }

    void render_request_line(const Request* value, IStream<byte>* stream)
    {
        SingleByteEncoder encoder;
        encoder.Initialize(Basic::globals->ascii_index, stream);

        value->resource->write_to_stream(&encoder, true, false);

        TextWriter text_writer(&encoder);
        text_writer.write_literal(" method ");
        value->method->write_to_stream(&encoder);
    }
}