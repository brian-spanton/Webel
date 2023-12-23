#include "stdafx.h"
#include "Http.Globals.h"
#include "Http.BodyChunksFrame.h"
#include "Http.Types.h"
#include "Basic.Event.h"

namespace Http
{
	using namespace Basic;

	void BodyChunksFrame::Initialize(IStream<byte>* body_stream)
	{
		__super::Initialize();
		this->body_stream = body_stream;
		this->size = 0;
		this->size_stream.Initialize(&this->size);
	}

	void BodyChunksFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::start_chunk_state:
			{
				byte b;
				if (!Event::ReadNext(event, &b, yield))
					return;

				if (b == Http::globals->CR)
				{
					switch_to_state(State::expecting_LF_after_size_state);
				}
				else
				{
					bool success = this->size_stream.WriteDigit(b);
					if (!success)
						switch_to_state(State::start_chunk_error);
				}
			}
			break;

		case State::expecting_LF_after_size_state:
			{
				byte b;
				if (!Event::ReadNext(event, &b, yield))
					return;

				if (b == Http::globals->LF)
				{
					if (this->size == 0)
					{
						switch_to_state(State::done_state);
					}
					else
					{
						chunk_frame.Initialize(this->body_stream, this->size);
						switch_to_state(State::chunk_frame_pending_state);
					}
				}
				else
				{
					Event::UndoReadNext(event);
					switch_to_state(State::expecting_LF_after_size_error);
				}
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
				byte b;
				if (!Event::ReadNext(event, &b, yield))
					return;

				if (b == Http::globals->CR)
				{
					switch_to_state(State::expecting_LF_after_chunk_state);
				}
				else
				{
					Event::UndoReadNext(event);
					switch_to_state(State::expecting_CR_after_chunk_error);
				}
			}
			break;

		case State::expecting_LF_after_chunk_state:
			{
				byte b;
				if (!Event::ReadNext(event, &b, yield))
					return;

				if (b == Http::globals->LF)
				{
					this->size = 0;
					this->size_stream.Initialize(&this->size);
					switch_to_state(State::start_chunk_state);
				}
				else
				{
					switch_to_state(State::expecting_LF_after_chunk_error);
				}
			}
			break;

		default:
			throw new Exception("BodyChunksFrame::Process unexpected state");
		}
	}
}