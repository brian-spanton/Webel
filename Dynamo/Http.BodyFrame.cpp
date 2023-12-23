#include "stdafx.h"
#include "Http.BodyFrame.h"
#include "Http.Globals.h"
#include "Basic.IgnoreFrame.h"
#include "Basic.FrameStream.h"

namespace Http
{
	using namespace Basic;

	void BodyFrame::Initialize(NameValueCollection* headers)
	{
		__super::Initialize();
		this->headers = headers;
	}

	void BodyFrame::set_body_stream(IStream<byte>* body_stream)
	{
		this->body_stream = body_stream;
	}

	void BodyFrame::switch_to_state(State state)
	{
		__super::switch_to_state(state);

		if (Succeeded())
			this->body_stream->WriteEOF();
	}

	void BodyFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::start_state:
			{
				if (this->body_stream.item() == 0)
				{
					IgnoreFrame::Ref ignore_frame = New<IgnoreFrame>();
					ignore_frame->Initialize(0xffffffff);

					FrameStream<byte>::Ref frame_stream = New<FrameStream<byte> >();
					frame_stream->Initialize(ignore_frame);

					this->body_stream = frame_stream;
				}

				UnicodeString::Ref contentType;
				bool success = this->headers->get_string(Http::globals->header_content_type, &contentType);
				if (!success)
				{
					switch_to_state(State::done_state);
					return;
				}

				UnicodeString::Ref contentEncoding;
				success = this->headers->get_string(Http::globals->header_content_encoding, &contentEncoding);
				if (success)
				{
					if (!contentEncoding.equals<false>(Http::globals->identity))
					{
						switch_to_state(State::unhandled_content_encoding_error);
						return;
					}
				}

				UnicodeString::Ref transferEncoding;
				success = this->headers->get_string(Http::globals->header_transfer_encoding, &transferEncoding);
				if (success)
				{
					if (transferEncoding.equals<false>(Http::globals->chunked))
					{
						this->chunks_frame.Initialize(this->body_stream);
						switch_to_state(State::chunks_frame_pending_state);
						return;
					}
					else if (!transferEncoding.equals<false>(Http::globals->identity))
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
					}
					else
					{
						this->chunk_frame.Initialize(this->body_stream, contentLength);
						switch_to_state(State::chunk_frame_pending_state);
					}
					return;
				}

				success = this->headers->get_base_10(Http::globals->header_content_length, &contentLength);
				if (success)
				{
					if (contentLength == 0)
					{
						switch_to_state(State::done_state);
					}
					else
					{
						this->chunk_frame.Initialize(this->body_stream, contentLength);
						switch_to_state(State::chunk_frame_pending_state);
					}
					return;
				}

				this->disconnect_frame.Initialize(this->body_stream);
				switch_to_state(State::disconnect_frame_pending_state);
			}
			break;

		case State::chunks_frame_pending_state:
			if (this->chunks_frame.Pending())
			{
				this->chunks_frame.Process(event, yield);
			}
			else if (this->chunks_frame.Failed())
			{
				switch_to_state(State::chunks_frame_failed);
			}
			else
			{
				this->headers_frame.Initialize(this->headers);
				switch_to_state(State::headers_frame_pending);
			}
			break;

		case State::chunk_frame_pending_state:
			if (this->chunk_frame.Pending())
			{
				this->chunk_frame.Process(event, yield);
			}
			else if (this->chunk_frame.Failed())
			{
				switch_to_state(State::chunk_frame_failed);
			}
			else
			{
				switch_to_state(State::done_state);
			}
			break;

		case State::disconnect_frame_pending_state:
			if (this->disconnect_frame.Pending())
			{
				this->disconnect_frame.Process(event, yield);
			}
			else if (this->disconnect_frame.Failed())
			{
				switch_to_state(State::disconnect_frame_failed);
			}
			else
			{
				switch_to_state(State::done_state);
			}
			break;

		case State::headers_frame_pending:
			if (this->headers_frame.Pending())
			{
				this->headers_frame.Process(event, yield);
			}
			else if (this->headers_frame.Failed())
			{
				switch_to_state(State::header_frame_failed);
			}
			else
			{
				switch_to_state(State::done_state);
			}
			break;

		default:
			throw new Exception("BodyFrame::Process unexpected state");
		}
	}
}