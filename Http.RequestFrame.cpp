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
        headers_frame(this->request->headers.get()),
        body_frame(this->request->headers)
    {
        this->resource_string->reserve(0x100);
    }

    EventResult RequestFrame::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::receiving_method_state:
            {
                byte b;
                EventResult result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

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
                EventResult result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

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
                EventResult result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

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
                EventResult result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

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
                EventResult result = delegate_event_change_state_on_fail(&this->headers_frame, event, State::headers_frame_failed);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                std::shared_ptr<CountStream<byte> > count_stream = std::make_shared<CountStream<byte> >();
                this->body_frame.set_decoded_content_stream(count_stream);

                switch_to_state(State::body_frame_pending_state);
            }
            break;

        case State::body_frame_pending_state:
            {
                EventResult result = delegate_event_change_state_on_fail(&this->body_frame, event, State::body_frame_failed);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                switch_to_state(State::done_state);
            }
            break;

        default:
            throw FatalError("Http::RequestFrame::handle_event unexpected state");
        }

        return EventResult::event_result_continue;
    }

    void render_request_line(const Request* value, IStream<byte>* stream)
    {
        SingleByteEncoder encoder;
        encoder.Initialize(Basic::globals->ascii_index, stream);

        value->method->write_to_stream(&encoder);

        stream->write_element(Http::globals->SP);
        value->resource->write_to_stream(&encoder, true, true);
    }
}