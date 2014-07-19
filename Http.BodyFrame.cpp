// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Http.BodyFrame.h"
#include "Http.Globals.h"
#include "Basic.IgnoreFrame.h"

namespace Http
{
    using namespace Basic;

    BodyFrame::BodyFrame(std::shared_ptr<NameValueCollection> headers) :
        headers(headers),
        headers_frame(this->headers.get()) // initialization is in order of declaration in class def
    {
    }

    void BodyFrame::set_body_stream(std::shared_ptr<IStream<byte> > body_stream)
    {
        this->body_stream = body_stream;
    }

    void BodyFrame::switch_to_state(State state)
    {
        __super::switch_to_state(state);

        if (succeeded())
        {
            this->body_stream->write_eof();
        }
    }

    void BodyFrame::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::start_state:
            {
                if (this->body_stream.get() == 0)
                {
                    this->body_stream = std::make_shared<IgnoreFrame<byte> >();
                }

                UnicodeStringRef contentType;
                bool success = this->headers->get_string(Http::globals->header_content_type, &contentType);
                if (!success)
                {
                    switch_to_state(State::done_state);
                    return;
                }

                UnicodeStringRef contentEncoding;
                success = this->headers->get_string(Http::globals->header_content_encoding, &contentEncoding);
                if (success)
                {
                    if (!equals<UnicodeString, false>(contentEncoding.get(), Http::globals->identity.get()))
                    {
                        switch_to_state(State::unhandled_content_encoding_error);
                        return;
                    }
                }

                UnicodeStringRef transferEncoding;
                success = this->headers->get_string(Http::globals->header_transfer_encoding, &transferEncoding);
                if (success)
                {
                    if (equals<UnicodeString, false>(transferEncoding.get(), Http::globals->chunked.get()))
                    {
                        this->chunks_frame = std::make_shared<BodyChunksFrame>(this->body_stream);
                        switch_to_state(State::chunks_frame_pending_state);
                        return;
                    }
                    else if (!equals<UnicodeString, false>(transferEncoding.get(), Http::globals->identity.get()))
                    {
                        switch_to_state(State::unhandled_transfer_encoding_error);
                        return;
                    }
                }

                uint32 contentLength;
                success = this->headers->get_base_10(Http::globals->header_transfer_length, &contentLength);
                if (success)
                {
                    if (contentLength == 0)
                    {
                        switch_to_state(State::done_state);
                        return;
                    }
                    else
                    {
                        this->chunk_frame = std::make_shared<LengthBodyFrame>(this->body_stream, contentLength);
                        switch_to_state(State::chunk_frame_pending_state);
                        return;
                    }
                }

                success = this->headers->get_base_10(Http::globals->header_content_length, &contentLength);
                if (success)
                {
                    if (contentLength == 0)
                    {
                        switch_to_state(State::done_state);
                        return;
                    }
                    else
                    {
                        this->chunk_frame = std::make_shared<LengthBodyFrame>(this->body_stream, contentLength);
                        switch_to_state(State::chunk_frame_pending_state);
                        return;
                    }
                }

                this->disconnect_frame = std::make_shared<DisconnectBodyFrame>(this->body_stream);
                switch_to_state(State::disconnect_frame_pending_state);
            }
            break;

        case State::chunks_frame_pending_state:
            delegate_event_change_state_on_fail(this->chunks_frame.get(), event, State::chunks_frame_failed);
            switch_to_state(State::headers_frame_pending);
            break;

        case State::chunk_frame_pending_state:
            delegate_event_change_state_on_fail(this->chunk_frame.get(), event, State::chunk_frame_failed);
            switch_to_state(State::done_state);
            break;

        case State::disconnect_frame_pending_state:
            delegate_event_change_state_on_fail(this->disconnect_frame.get(), event, State::disconnect_frame_failed);
            switch_to_state(State::done_state);
            break;

        case State::headers_frame_pending:
            delegate_event_change_state_on_fail(&this->headers_frame, event, State::header_frame_failed);
            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("BodyFrame::handle_event unexpected state");
        }
    }
}