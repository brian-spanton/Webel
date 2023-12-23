#include "stdafx.h"
#include "Http.DisconnectBodyFrame.h"
#include "Basic.Event.h"

namespace Http
{
	using namespace Basic;

	void DisconnectBodyFrame::Initialize(IStream<byte>* body_stream)
	{
		__super::Initialize();
		this->body_stream = body_stream;
	}

	void DisconnectBodyFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::receiving_body_state:
			if (event->get_type() == EventType::element_stream_ending_event)
			{
				switch_to_state(State::done_state);
			}
			else
			{
				const byte* elements;
				uint32 count;

				if (!Event::Read(event, 0xffffffff, &elements, &count, yield))
					return;

				this->body_stream->Write(elements, count);
				(*yield) = true;
			}
			break;

		default:
			throw new Exception("Http::DisconnectBodyFrame::Process unexpected state");
		}
	}
}