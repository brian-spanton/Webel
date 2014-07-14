// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Service.StandardSingleByteEncoding.h"
#include "Http.Globals.h"
#include "Service.Globals.h"

namespace Service
{
    StandardSingleByteEncoding::StandardSingleByteEncoding(std::shared_ptr<SingleByteEncodingIndex> index) :
        client(std::make_shared<Web::Client>()),
        index(index),
        pointer_stream(&this->pointer),
        codepoint_stream(&this->codepoint)
    {
    }

    void StandardSingleByteEncoding::start(std::shared_ptr<Uri> index_url)
    {
        this->client->Get(index_url, this->shared_from_this(), ByteStringRef());
    }

    void StandardSingleByteEncoding::consider_event(IEvent* event)
    {
        if (event->get_type() == Http::EventType::response_complete_event)
        {
            switch_to_state(State::connection_lost_error);
            return;
        }

        if (event->get_type() == Basic::EventType::element_stream_ending_event)
        {
            if (this->get_state() != State::line_start_state)
            {
                switch_to_state(State::malformed_content_error);
                return;
            }

            switch_to_state(State::done_state);
            return;
        }

        switch (get_state())
        {
        case State::headers_pending_state:
            {
                if (event->get_type() != Http::EventType::response_headers_event)
                    throw new FatalError("unexpected event");

                std::shared_ptr<Http::Response> response = this->client->history.back().response;
                if (response->code != 200)
                {
                    std::shared_ptr<Uri> url;
                    this->client->get_url(&url);

                    url->write_to_stream(Service::globals->LogStream(), 0, 0);
                    Service::globals->DebugWriter()->WriteLine(" did not return 200");

                    switch_to_state(State::done_state);
                    return;
                }

                std::shared_ptr<FrameStream<byte> > frame_stream = std::make_shared<FrameStream<byte> >();
                frame_stream->Initialize(this);

                this->client->set_body_stream(frame_stream);

                switch_to_state(State::line_start_state);
                throw Yield("event consumed");
            }
            break;

        case State::line_start_state:
            {
                byte b;
                Event::ReadNext(event, &b);

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
                    Event::UndoReadNext(event);
                    this->pointer_stream.reset();
                    switch_to_state(State::index_pending_state);
                }
                else
                {
                    switch_to_state(State::line_start_error);
                }
            }
            break;

        case State::ignore_line_state:
            {
                byte b;
                Event::ReadNext(event, &b);

                if (b == Http::globals->LF)
                {
                    switch_to_state(State::line_start_state);
                }
            }
            break;

        case State::before_index_state:
            {
                byte b;
                Event::ReadNext(event, &b);

                if (Http::globals->WSP[b])
                {
                }
                else if (Http::globals->DIGIT[b])
                {
                    Event::UndoReadNext(event);
                    this->pointer_stream.reset();
                    switch_to_state(State::index_pending_state);
                }
                else
                {
                    switch_to_state(State::before_index_error);
                }
            }
            break;

        case State::index_pending_state:
            {
                byte b;
                Event::ReadNext(event, &b);

                bool success = this->pointer_stream.WriteDigit(b);
                if (!success)
                {
                    if (this->pointer_stream.get_digit_count() == 0)
                    {
                        switch_to_state(State::index_pending_error);
                    }
                    else
                    {
                        Event::UndoReadNext(event);
                        switch_to_state(State::before_codepoint_state);
                    }
                }
            }
            break;

        case State::before_codepoint_state:
            {
                byte b;
                Event::ReadNext(event, &b);

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
                byte b;
                Event::ReadNext(event, &b);

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
            throw FatalError("Basic::StandardSingleByteEncoding::Complete unexpected state");
        }
    }
}