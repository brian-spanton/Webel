// Copyright � 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Event.h"
#include "Basic.Globals.h"
#include "Basic.TextWriter.h"
#include "Basic.IgnoreFrame.h"
#include "Http.ResponseFrame.h"
#include "Http.Globals.h"
#include "Http.LengthBodyFrame.h"
#include "Http.BodyChunksFrame.h"
#include "Http.DisconnectBodyFrame.h"

namespace Http
{
    using namespace Basic;

    ResponseFrame::ResponseFrame(std::shared_ptr<Transaction> transaction, std::shared_ptr<IProcess> call_back, std::shared_ptr<void> context) :
        transaction(transaction),
        call_back(call_back),
        context(context),
        number_stream(&transaction->response->code), // initialization is in order of declaration in class def
        headers_frame(transaction->response->headers.get()) // initialization is in order of declaration in class def
    {
    }

    ProcessResult ResponseFrame::process_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::receiving_protocol_state:
            {
                byte b;
                ProcessResult result = Event::ReadNext(event, &b);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                if (b == Http::globals->SP)
                {
                    switch_to_state(State::receiving_code_state);
                }
                else
                {
                    this->transaction->response->protocol->push_back(b);
                }
            }
            break;

        case State::receiving_code_state:
            {
                byte b;
                ProcessResult result = Event::ReadNext(event, &b);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                if (b == Http::globals->SP)
                {
                    if (this->number_stream.get_digit_count() == 0 || this->transaction->response->code < 100 || this->transaction->response->code > 599)
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
                ProcessResult result = Event::ReadNext(event, &b);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                if (b == Http::globals->CR)
                {
                    switch_to_state(State::expecting_LF_after_reason_state);
                }
                else if (b == Http::globals->SP || b == Http::globals->HT)
                {
                    this->transaction->response->reason->push_back(b);
                }
                else if (Http::globals->CTL[b])
                {
                    switch_to_state(State::receiving_reason_error);
                }
                else
                {
                    this->transaction->response->reason->push_back(b);
                }
            }
            break;

        case State::expecting_LF_after_reason_state:
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
                    switch_to_state(State::expecting_LF_after_reason_error);
                }
            }
            break;

        case State::headers_frame_pending_state:
            {
                ProcessResult result = process_event_change_state_on_fail(&this->headers_frame, event, State::headers_frame_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

			    std::shared_ptr<LogEntry> entry = std::make_shared<LogEntry>(LogLevel::Debug, "Http", "ResponseFrame", "process_event");
			    TextWriter(&entry->unicode_message).write_literal("Response headers received: ");
                this->transaction->response->render_response_line(&entry->unicode_message);
			    Basic::globals->add_entry(entry);

                std::shared_ptr<IProcess> call_back = this->call_back.lock();
                if (call_back)
                {
                    Http::ResponseHeadersEvent event;
                    event.context = this->context;
                    process_event_ignore_failures(call_back.get(), &event);
                }

                if (!this->decoded_content_stream)
                    this->decoded_content_stream = std::make_shared<IgnoreFrame<byte> >();

                // get the body frame set up first, because the ResponseHeadersEvent call_back can recurse into
                // this class to set the decoded content stream on the body frame

                uint16 code = this->transaction->response->code;
                bool body_expected = !(code / 100 == 1 || code == 204 || code == 205 || code == 304 || equals<UnicodeString, true>(transaction->request->method.get(), Http::globals->head_method.get()));
                if (!body_expected)
                {
                    switch_to_state(State::done_state);
                    return ProcessResult::process_result_ready;
                }

                BodyFrame::make_body_frame(this->decoded_content_stream, this->transaction->response->headers.get(), &this->body_frame);
                if (!this->body_frame)
                {
                    switch_to_state(State::done_state);
                    return ProcessResult::process_result_ready;
                }

                switch_to_state(State::body_pending_state);
                return ProcessResult::process_result_ready;
            }
            break;

        case State::body_pending_state:
            {
                ProcessResult result = process_event_change_state_on_fail(this->body_frame.get(), event, State::body_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                switch_to_state(State::done_state);
            }
            break;

        default:
            throw FatalError("Http", "ResponseFrame", "process_event", "unhandled state", this->get_state());
        }

        return ProcessResult::process_result_ready;
    }

    void ResponseFrame::set_decoded_content_stream(std::shared_ptr<IStream<byte> > decoded_content_stream)
    {
        this->decoded_content_stream = decoded_content_stream;
    }

    void Response::render_response_line(IStream<Codepoint>* stream)
    {
        TextWriter writer(stream);

        this->protocol->write_to_stream(stream);
        writer.WriteFormat<0x10>(" %d ", this->code);
        this->reason->write_to_stream(stream);
    }

    void Response::render_response_line(IStream<byte>* stream)
    {
        SingleByteEncoder encoder;
        encoder.Initialize(Basic::globals->ascii_index, stream);

        render_response_line(&encoder);
    }
}