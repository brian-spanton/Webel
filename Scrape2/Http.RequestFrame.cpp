// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Http.RequestFrame.h"
#include "Http.Globals.h"
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

	ConsumeElementsResult RequestFrame::consume_elements(IElementSource<byte>* element_source)
    {
        switch (get_state())
        {
		case State::receiving_method_state:
        {
            byte b;

            bool success = element_source->ReadNext(&b);
			if (!success)
				return ConsumeElementsResult::in_progress;

            if (b == Http::globals->SP)
            {
                switch_to_state(State::receiving_resource_state);
				return ConsumeElementsResult::in_progress;
			}
            else if (Http::globals->TOKEN[b])
            {
                this->request->method->push_back(b);
				return ConsumeElementsResult::in_progress;
            }
            else
            {
                switch_to_state(State::receiving_method_error);
				return ConsumeElementsResult::failed;
            }
        }

        case State::receiving_resource_state:
        {
			byte b;

			bool success = element_source->ReadNext(&b);
			if (!success)
				return ConsumeElementsResult::in_progress;

            if (b == Http::globals->SP)
            {
                this->resource_decoder.write_eof();

                Uri base;
                base.scheme = Basic::globals->http_scheme;

                this->request->resource->Parse(this->resource_string.get(), &base);

                switch_to_state(State::receiving_protocol_state);
				return ConsumeElementsResult::in_progress;
			}
            else
            {
                this->resource_decoder.write_element(b);
				return ConsumeElementsResult::in_progress;
			}
        }

		case State::receiving_protocol_state:
		{
			byte b;

			bool success = element_source->ReadNext(&b);
			if (!success)
				return ConsumeElementsResult::in_progress;

			if (b == Http::globals->CR)
			{
				switch_to_state(State::expecting_LF_after_protocol_state);
				return ConsumeElementsResult::in_progress;
			}
			else
			{
				this->request->protocol->push_back(b);
				return ConsumeElementsResult::in_progress;
			}
		}

		case State::expecting_LF_after_protocol_state:
		{
			byte b;

			bool success = element_source->ReadNext(&b);
			if (!success)
				return ConsumeElementsResult::in_progress;

			if (b == Http::globals->LF)
			{
				switch_to_state(State::headers_frame_pending_state);
				return ConsumeElementsResult::in_progress;
			}
			else
			{
				switch_to_state(State::expecting_LF_after_protocol_error);
				return ConsumeElementsResult::failed;
			}
		}

		case State::headers_frame_pending_state:
		{
			byte b;

			bool success = element_source->ReadNext(&b);
			if (!success)
				return ConsumeElementsResult::in_progress;

			this->headers_frame.write_element(b);

			if (this->headers_frame.in_progress())
				return ConsumeElementsResult::in_progress;

			if (this->headers_frame.failed())
			{
				switch_to_state(State::headers_frame_failed);
				return ConsumeElementsResult::failed;
			}

			std::shared_ptr<CountStream<byte> > count_stream = std::make_shared<CountStream<byte> >();
			this->body_frame.set_body_stream(count_stream);

			switch_to_state(State::body_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}

		case State::body_frame_pending_state:
		{
			byte b;

			ConsumeElementsResult result = Basic::consume_elements(&this->body_frame, element_source, this, State::body_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			switch_to_state(State::done_state);
			return ConsumeElementsResult::succeeded;
		}

		default:
			throw FatalError("Http::RequestFrame::handle_event unexpected state");
        }
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