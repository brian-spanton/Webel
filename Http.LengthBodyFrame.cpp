// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Http.LengthBodyFrame.h"
#include "Basic.Event.h"

namespace Http
{
	using namespace Basic;

	void LengthBodyFrame::Initialize(IStream<byte>* body_stream, uint32 bytes_expected)
	{
		__super::Initialize();
		this->body_stream = body_stream;
		this->bytes_expected = bytes_expected;
		this->bytes_received = 0;

		if (this->bytes_expected == 0)
			switch_to_state(State::done_state);
	}

	void LengthBodyFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::receiving_body_state:
			{
				const byte* elements;
				uint32 useable;

				if (!Event::Read(event, this->bytes_expected - this->bytes_received, &elements, &useable, yield))
					return;

				this->body_stream->Write(elements, useable);

				this->bytes_received += useable;

				if (this->bytes_received != this->bytes_expected)
				{
					(*yield) = true;
				}
				else
				{
					switch_to_state(State::done_state);
				}
			}
			break;

		default:
			throw new Exception("Http::LengthBodyFrame::Process unexpected state");
		}
	}
}