#include "stdafx.h"
#include "Basic.Event.h"
#include "Http.ResponseHeadersFrame.h"
#include "Basic.Globals.h"
#include "Http.Globals.h"
#include "Basic.TextWriter.h"

namespace Http
{
	using namespace Basic;

	void ResponseHeadersFrame::Initialize(UnicodeString* method, Response* response)
	{
		__super::Initialize();
		this->method = method;
		this->response = response;
		this->number_stream.Initialize(&this->response->code);
		this->headers_frame.Initialize(this->response->headers);
	}

	void ResponseHeadersFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::receiving_protocol_state:
			{
				byte b;
				if (!Event::ReadNext(event, &b, yield))
					return;

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
				if (!Event::ReadNext(event, &b, yield))
					return;

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
						Event::UndoReadNext(event);
						switch_to_state(State::write_to_number_stream_failed);
					}
				}
			}
			break;

		case State::receiving_reason_state:
			{
				byte b;
				if (!Event::ReadNext(event, &b, yield))
					return;

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
					Event::UndoReadNext(event);
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
				if (!Event::ReadNext(event, &b, yield))
					return;

				if (b == Http::globals->LF)
				{
					switch_to_state(State::headers_frame_pending_state);
				}
				else
				{
					Event::UndoReadNext(event);
					switch_to_state(State::expecting_LF_after_reason_error);
				}
			}
			break;

		case State::headers_frame_pending_state:
			if (this->headers_frame.Pending())
			{
				this->headers_frame.Process(event, yield);
			}
			else if (this->headers_frame.Failed())
			{
				switch_to_state(State::headers_frame_failed);
			}
			else
			{
				switch_to_state(State::done_state);
			}
			break;

		default:
			throw new Exception("ResponseHeadersFrame::Process unexpected state");
		}
	}

	void ResponseHeadersFrame::WriteResponseLineTo(IStream<byte>* stream)
	{
		this->response->protocol->ascii_encode(stream);

		stream->Write(&Http::globals->SP, 1);

		UnicodeString::Ref code = New<UnicodeString>();
		TextWriter writer(code);
		writer.WriteFormat<0x10>("%d", this->response->code);

		code->ascii_encode(stream);

		stream->Write(&Http::globals->SP, 1);

		this->response->reason->ascii_encode(stream);
	}

	void ResponseHeadersFrame::SerializeTo(IStream<byte>* stream)
	{
		WriteResponseLineTo(stream);

		stream->Write(Http::globals->CRLF, _countof(Http::globals->CRLF));

		Inline<HeadersFrame> frame;
		frame.Initialize(this->response->headers);

		frame.SerializeTo(stream);

		if (this->response->server_body.item() != 0)
			this->response->server_body->SerializeTo(stream);
	}
}