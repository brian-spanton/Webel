// Copyright � 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Event.h"
#include "Http.ResponseHeadersFrame.h"
#include "Basic.Globals.h"
#include "Http.Globals.h"
#include "Basic.TextWriter.h"

namespace Http
{
    using namespace Basic;

    ResponseHeadersFrame::ResponseHeadersFrame(UnicodeStringRef method, Response* response) :
        method(method),
        response(response),
        number_stream(&this->response->code), // initialization is in order of declaration in class def
        headers_frame(this->response->headers.get()) // initialization is in order of declaration in class def
    {
    }

    event_result ResponseHeadersFrame::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::receiving_protocol_state:
            {
                byte b;
                event_result result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return event_result_yield;

                if (b == Http::globals->SP)
                {
                    switch_to_state(State::receiving_code_state);
                }
                else
                {
                    this->response->protocol->push_back(b);
                }
            }
            break;

        case State::receiving_code_state:
            {
                byte b;
                event_result result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return event_result_yield;

                if (b == Http::globals->SP)
                {
                    if (this->number_stream.get_digit_count() == 0 || this->response->code < 100 || this->response->code > 599)
                    {
                        switch_to_state(State::receiving_code_error);
                    }
                    else
                    {
                        switch_to_state(State::receiving_reason_state);
                    }
                }
                else if (b == Http::globals->CR)
                {
                    switch_to_state(State::expecting_LF_after_reason_state);
                }
                else
                {
                    bool success = this->number_stream.WriteDigit(b);
                    if (!success)
                    {
                        switch_to_state(State::write_to_number_stream_failed);
                    }
                }
            }
            break;

        case State::receiving_reason_state:
            {
                byte b;
                event_result result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return event_result_yield;

                if (b == Http::globals->CR)
                {
                    switch_to_state(State::expecting_LF_after_reason_state);
                }
                else if (b == Http::globals->SP || b == Http::globals->HT)
                {
                    this->response->reason->push_back(b);
                }
                else if (Http::globals->CTL[b])
                {
                    switch_to_state(State::receiving_reason_error);
                }
                else
                {
                    this->response->reason->push_back(b);
                }
            }
            break;

        case State::expecting_LF_after_reason_state:
            {
                byte b;
                event_result result = Event::ReadNext(event, &b);
                if (result == event_result_yield)
                    return event_result_yield;

                if (b == Http::globals->LF)
                {
                    switch_to_state(State::headers_frame_pending_state);
                }
                else
                {
                    switch_to_state(State::expecting_LF_after_reason_error);
                }
            }
            break;

        case State::headers_frame_pending_state:
            {
                event_result result = delegate_event_change_state_on_fail(&this->headers_frame, event, State::headers_frame_failed);
                if (result == event_result_yield)
                    return event_result_yield;

                switch_to_state(State::done_state);
            }
            break;

        default:
            throw FatalError("ResponseHeadersFrame::handle_event unexpected state");
        }

        return event_result_continue;
    }

    void render_response_line(const Response* value, IStream<byte>* stream)
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
    }
}