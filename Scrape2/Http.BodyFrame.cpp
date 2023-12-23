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

        if (this->succeeded())
        {
            this->body_stream->write_eof();
        }
    }

	ConsumeElementsResult BodyFrame::consume_elements(IElementSource<byte>* element_source)
    {
        switch (this->get_state())
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
                    return ConsumeElementsResult::succeeded;
                }

                UnicodeStringRef contentEncoding;
                success = this->headers->get_string(Http::globals->header_content_encoding, &contentEncoding);
                if (success)
                {
                    if (!equals<UnicodeString, false>(contentEncoding.get(), Http::globals->identity.get()))
                    {
                        switch_to_state(State::unhandled_content_encoding_error);
                        return ConsumeElementsResult::failed;
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
                        return ConsumeElementsResult::in_progress;
                    }
                    else if (!equals<UnicodeString, false>(transferEncoding.get(), Http::globals->identity.get()))
                    {
                        switch_to_state(State::unhandled_transfer_encoding_error);
                        return ConsumeElementsResult::failed;
                    }
                }

                uint32 contentLength;
                success = this->headers->get_base_10(Http::globals->header_transfer_length, &contentLength);
                if (success)
                {
                    if (contentLength == 0)
                    {
                        switch_to_state(State::done_state);
                        return ConsumeElementsResult::succeeded;
                    }
                    else
                    {
                        this->chunk_frame = std::make_shared<LengthBodyFrame>(this->body_stream, contentLength);
                        switch_to_state(State::chunk_frame_pending_state);
                        return ConsumeElementsResult::in_progress;
                    }
                }

                success = this->headers->get_base_10(Http::globals->header_content_length, &contentLength);
                if (success)
                {
                    if (contentLength == 0)
                    {
                        switch_to_state(State::done_state);
                        return ConsumeElementsResult::succeeded;
                    }
                    else
                    {
                        this->chunk_frame = std::make_shared<LengthBodyFrame>(this->body_stream, contentLength);
                        switch_to_state(State::chunk_frame_pending_state);
                        return ConsumeElementsResult::in_progress;
                    }
                }

                switch_to_state(State::disconnect_frame_pending_state);
                return ConsumeElementsResult::in_progress;
            }
            break;

        case State::chunks_frame_pending_state:
            {
                bool success = this->chunks_frame->write_elements(element_source);
                if (!success)
                    return ConsumeElementsResult::failed;

                if (this->chunks_frame->failed())
                {
                    switch_to_state(State::chunks_frame_failed);
                    return ConsumeElementsResult::failed;
                }

                switch_to_state(State::headers_frame_pending);
                return ConsumeElementsResult::in_progress;
            }

        case State::chunk_frame_pending_state:
            ConsumeElementsResult result = Basic::consume_elements(this->chunk_frame.get(), element_source, this, State::chunk_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			switch_to_state(State::done_state);
            return true;

        case State::disconnect_frame_pending_state:
            ConsumeElementsResult result = Basic::consume_elements(this->disconnect_frame.get(), element_source, this, State::disconnect_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			switch_to_state(State::done_state);
            return true;

        case State::headers_frame_pending:
            ConsumeElementsResult result = Basic::consume_elements(&this->headers_frame, element_source, this, State::header_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			switch_to_state(State::done_state);
            return true;

        default:
            throw FatalError("BodyFrame::handle_event unexpected state");
        }
    }
}