// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Http.RequestFrame.h"
#include "Http.HeadersFrame.h"
#include "Http.Globals.h"
#include "Basic.Event.h"
#include "Basic.CountStream.h"
#include "Basic.Globals.h"
#include "Basic.SingleByteEncodingIndex.h"
#include "Basic.SingleByteEncoder.h"

namespace Http
{
	using namespace Basic;

	void RequestFrame::Initialize(Request* request)
	{
		__super::Initialize();
		this->request = request;

		this->resource_string = New<UnicodeString>();
		this->resource_string->reserve(0x100);

		this->resource_decoder.Initialize(Basic::globals->ascii_index, this->resource_string);

		this->headers_frame.Initialize(this->request->headers);
	}

	void RequestFrame::Process(IEvent* event, bool* yield)
	{
		(*yield) = false; // $ i've been defaulting to true elsewhere

		switch (frame_state())
		{
		case State::receiving_method_state:
			{
				byte b;
				if (!Event::ReadNext(event, &b, yield))
					return;

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
					Event::UndoReadNext(event);
					switch_to_state(State::receiving_method_error);
				}
			}
			break;

		case State::receiving_resource_state:
			{
				byte b;
				if (!Event::ReadNext(event, &b, yield))
					return;

				if (b == Http::globals->SP)
				{
					this->resource_decoder.WriteEOF();

					Inline<Uri> base;
					base.scheme = Basic::globals->http_scheme;

					this->request->resource->Parse(this->resource_string, &base);
					switch_to_state(State::receiving_protocol_state);
				}
				else
				{
					this->resource_decoder.Write(&b, 1);
				}
			}
			break;

		case State::receiving_protocol_state:
			{
				byte b;
				if (!Event::ReadNext(event, &b, yield))
					return;

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
				if (!Event::ReadNext(event, &b, yield))
					return;

				if (b == Http::globals->LF)
				{
					switch_to_state(State::headers_frame_pending_state);
				}
				else
				{
					Event::UndoReadNext(event);
					switch_to_state(State::expecting_LF_after_protocol_error);
				}
			}
			break;

		case State::headers_frame_pending_state:
			if (this->headers_frame.Pending())
			{
				this->headers_frame.Process(event, yield);
			}
			
			if (this->headers_frame.Failed())
			{
				switch_to_state(State::headers_frame_failed);
			}
			else if (this->headers_frame.Succeeded())
			{
				this->body_frame.Initialize(this->request->headers);

				CountStream<byte>::Ref count_stream = New<CountStream<byte> >();
				this->body_frame.set_body_stream(count_stream);

				switch_to_state(State::body_frame_pending_state);
			}
			break;

		case State::body_frame_pending_state:
			if (this->body_frame.Pending())
			{
				this->body_frame.Process(event, yield);
			}

			if (this->body_frame.Failed())
			{
				switch_to_state(State::body_frame_failed);
			}
			else if (this->body_frame.Succeeded())
			{
				switch_to_state(State::done_state);
			}
			break;

		default:
			throw new Exception("Http::RequestFrame::Process unexpected state");
		}
	}

	void RequestFrame::WriteRequestLineTo(IStream<byte>* stream)
	{
		this->request->method->ascii_encode(stream);

		stream->Write(&Http::globals->SP, 1);

		Inline<SingleByteEncoder> encoder;
		encoder.Initialize(Basic::globals->ascii_index, stream);

		this->request->resource->SerializeTo(&encoder, true, true);

		stream->Write(&Http::globals->SP, 1);

		this->request->protocol->ascii_encode(stream);
	}

	void RequestFrame::SerializeTo(IStream<byte>* stream)
	{
		WriteRequestLineTo(stream);

		stream->Write(Http::globals->CRLF, _countof(Http::globals->CRLF));

		this->headers_frame.SerializeTo(stream);

		if (this->request->client_body.item() != 0)
			this->request->client_body->SerializeTo(stream);
	}
}