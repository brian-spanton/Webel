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

    void BodyFrame::set_decoded_content_stream(std::shared_ptr<IStream<byte> > decoded_content_stream)
    {
        this->decoded_content_stream = decoded_content_stream;
    }

    void BodyFrame::switch_to_state(State state)
    {
        __super::switch_to_state(state);

        if (succeeded())
        {
            this->decoded_content_stream->write_eof();
        }
    }

    EventResult BodyFrame::consider_event(IEvent* event)
    {
        EventResult result;

        switch (get_state())
        {
        case State::start_state:
            {
                if (this->decoded_content_stream.get() == 0)
                {
                    this->decoded_content_stream = std::make_shared<IgnoreFrame<byte> >();
                }

                UnicodeStringRef contentType;
                bool success = this->headers->get_string(Http::globals->header_content_type, &contentType);
                if (!success)
                {
                    // content-type can be missing if there is no body, for instance in a typical a 3xx response
                    switch_to_state(State::done_state);
                    return EventResult::event_result_continue;
                }

                UnicodeStringRef contentEncoding;
                success = this->headers->get_string(Http::globals->header_content_encoding, &contentEncoding);
                if (success)
                {
                    if (equals<UnicodeString, false>(contentEncoding.get(), Http::globals->gzip.get()))
                    {
                        // RFC1952 https://www.rfc-editor.org/rfc/rfc1952

                        switch_to_state(State::unhandled_content_encoding_error);
                        
                        HandleError("content encoding gzip NYI");

                        //this->decoded_content_stream = std::make_shared<Gzip<byte> >();

                        return EventResult::event_result_continue;
                    }
                    else if (!equals<UnicodeString, false>(contentEncoding.get(), Http::globals->identity.get()))
                    {
                        switch_to_state(State::unhandled_content_encoding_error);

                        ByteString error;
                        error.append((byte*)"unhandled content encoding=");
                        ascii_encode(contentEncoding.get(), &error);

                        HandleError((char*)error.c_str());

                        return EventResult::event_result_continue;
                    }
                }

                UnicodeStringRef transferEncoding;
                success = this->headers->get_string(Http::globals->header_transfer_encoding, &transferEncoding);
                if (success)
                {
                    if (equals<UnicodeString, false>(transferEncoding.get(), Http::globals->chunked.get()))
                    {
                        this->chunks_frame = std::make_shared<BodyChunksFrame>(this->decoded_content_stream);
                        switch_to_state(State::chunks_frame_pending_state);
                        return EventResult::event_result_continue;
                    }
                    else if (!equals<UnicodeString, false>(transferEncoding.get(), Http::globals->identity.get()))
                    {
                        switch_to_state(State::unhandled_transfer_encoding_error);
                        return EventResult::event_result_continue;
                    }
                }

                // $ why does transfer-length take precedence over content-length? if we have both what should we do?
                uint32 contentLength;
                success = this->headers->get_base_10(Http::globals->header_transfer_length, &contentLength);
                if (success)
                {
                    if (contentLength == 0)
                    {
                        switch_to_state(State::done_state);
                        return EventResult::event_result_continue;
                    }
                    else
                    {
                        this->chunk_frame = std::make_shared<LengthBodyFrame>(this->decoded_content_stream, contentLength);
                        switch_to_state(State::chunk_frame_pending_state);
                        return EventResult::event_result_continue;
                    }
                }

                success = this->headers->get_base_10(Http::globals->header_content_length, &contentLength);
                if (success)
                {
                    if (contentLength == 0)
                    {
                        switch_to_state(State::done_state);
                        return EventResult::event_result_continue;
                    }
                    else
                    {
                        this->chunk_frame = std::make_shared<LengthBodyFrame>(this->decoded_content_stream, contentLength);
                        switch_to_state(State::chunk_frame_pending_state);
                        return EventResult::event_result_continue;
                    }
                }

                // if there is no content length, then the body is terminated by transport disconnect
                this->disconnect_frame = std::make_shared<DisconnectBodyFrame>(this->decoded_content_stream);
                switch_to_state(State::disconnect_frame_pending_state);
            }
            break;

        case State::chunks_frame_pending_state:
            result = delegate_event_change_state_on_fail(this->chunks_frame.get(), event, State::chunks_frame_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::headers_frame_pending);
            break;

        case State::chunk_frame_pending_state:
            result = delegate_event_change_state_on_fail(this->chunk_frame.get(), event, State::chunk_frame_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::done_state);
            break;

        case State::disconnect_frame_pending_state:
            result = delegate_event_change_state_on_fail(this->disconnect_frame.get(), event, State::disconnect_frame_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::done_state);
            break;

        case State::headers_frame_pending:
            result = delegate_event_change_state_on_fail(&this->headers_frame, event, State::header_frame_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            // $ I don't remember why this is getting headers at the end after a chunked body, check the RFC
            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("BodyFrame::handle_event unexpected state");
        }

        return EventResult::event_result_continue;
    }
}