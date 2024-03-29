// Copyright � 2013 Brian Spanton

#include "stdafx.h"
#include "Service.StandardSingleByteEncoding.h"
#include "Http.Globals.h"
#include "Service.Globals.h"

namespace Service
{
    StandardSingleByteEncoding::StandardSingleByteEncoding(std::shared_ptr<SingleByteEncodingIndex> index) :
        client(std::make_shared<Web::Client>()),
        index(index),
        pointer_stream(&this->pointer), // initialization is in order of declaration in class def
        codepoint_stream(&this->codepoint) // initialization is in order of declaration in class def
    {
    }

    void StandardSingleByteEncoding::start(std::shared_ptr<Uri> index_url)
    {
        this->self = this->shared_from_this();
        this->client->Get(index_url, 0, this->self, ByteStringRef());
    }

    void StandardSingleByteEncoding::switch_to_state(State state)
    {
        __super::switch_to_state(state);

        if (!this->in_progress())
        {
            // so we don't leak ourself
            this->self.reset();
        }
    }

    ProcessResult StandardSingleByteEncoding::process_event(IEvent* event)
    {
        if (event->get_type() == Http::EventType::response_complete_event)
        {
            std::shared_ptr<Uri> url;
            this->client->get_url(&url);

			std::shared_ptr<LogEntry> entry = std::make_shared<LogEntry>(LogLevel::Info, "Service", "StandardSingleByteEncoding", "process_event");
			url->write_to_stream(&entry->unicode_message, 0, 0);
			Basic::globals->add_entry(entry);

            switch_to_state(State::connection_lost_error);
            return ProcessResult::process_result_ready;
        }

        switch (get_state())
        {
        case State::headers_pending_state:
            {
                if (event->get_type() != Http::EventType::response_headers_event)
                {
                    StateMachine::LogUnexpectedEvent("Service", "StandardSingleByteEncoding", "process_event", event);
                    switch_to_state(State::unexpected_event_error);
                    return ProcessResult::process_result_blocked;
                }

                if (this->client->transaction->response->code != 200)
                {
                    switch_to_state(State::done_state);
                    return ProcessResult::process_result_blocked;
                }

                this->client->set_decoded_content_stream(this->shared_from_this());

                switch_to_state(State::line_start_state);
                return ProcessResult::process_result_blocked;
            }
            break;

        default:
            throw FatalError("Service", "StandardSingleByteEncoding", "process_event", "unhandled state", this->get_state());
        }
    }

    void StandardSingleByteEncoding::write_element(byte b)
    {
        switch (get_state())
        {
        case State::line_start_state:
            {
                if (b == '#')
                {
                    switch_to_state(State::ignore_line_state);
                }
                else if (b == Http::globals->LF)
                {
                }
                else if (Http::globals->WSP[b])
                {
                    switch_to_state(State::before_index_state);
                }
                else if (Http::globals->DIGIT[b])
                {
                    this->pointer_stream.reset();
                    switch_to_state(State::index_pending_state);
                    write_element(b);
                    return;
                }
                else
                {
                    switch_to_state(State::line_start_error);
                }
            }
            break;

        case State::ignore_line_state:
            {
                if (b == Http::globals->LF)
                {
                    switch_to_state(State::line_start_state);
                }
            }
            break;

        case State::before_index_state:
            {
                if (Http::globals->WSP[b])
                {
                }
                else if (Http::globals->DIGIT[b])
                {
                    this->pointer_stream.reset();
                    switch_to_state(State::index_pending_state);
                    write_element(b);
                    return;
                }
                else
                {
                    switch_to_state(State::before_index_error);
                }
            }
            break;

        case State::index_pending_state:
            {
                bool success = this->pointer_stream.WriteDigit(b);
                if (!success)
                {
                    if (this->pointer_stream.get_digit_count() == 0)
                    {
                        switch_to_state(State::index_pending_error);
                    }
                    else
                    {
                        switch_to_state(State::before_codepoint_state);
                        write_element(b);
                        return;
                    }
                }
            }
            break;

        case State::before_codepoint_state:
            {
                if (Http::globals->WSP[b])
                {
                }
                else if (b == '0')
                {
                }
                else if (b == 'x')
                {
                    this->codepoint_stream.reset();
                    switch_to_state(State::codepoint_pending_state);
                }
                else
                {
                    switch_to_state(State::before_codepoint_error);
                }
            }
            break;

        case State::codepoint_pending_state:
            {
                bool success = this->codepoint_stream.WriteDigit(b);
                if (!success)
                {
                    if (this->codepoint_stream.get_digit_count() == 0)
                    {
                        switch_to_state(State::codepoint_pending_error);
                    }
                    else
                    {
                        this->index->pointer_map[this->pointer] = this->codepoint;
                        this->index->codepoint_map.insert(SingleByteEncodingIndex::CodepointMap::value_type(this->codepoint, this->pointer));
                        switch_to_state(State::ignore_line_state);
                    }
                }
            }
            break;

        default:
            throw FatalError("Service", "StandardSingleByteEncoding", "write_element", "unhandled state", get_state());
        }
    }

    void StandardSingleByteEncoding::write_eof()
    {
        if (this->get_state() != State::line_start_state)
        {
            switch_to_state(State::malformed_content_error);
            return;
        }

        switch_to_state(State::done_state);
    }
}