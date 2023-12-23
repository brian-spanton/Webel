#include "stdafx.h"
#include "Basic.IgnoreFrame.h"
#include "Basic.Event.h"

namespace Basic
{
	void IgnoreFrame::Initialize(uint32 expected)
	{
		__super::Initialize();
		this->expected = expected;
		this->received = 0;
	}

	void IgnoreFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::receiving_state:
			{
				const byte* elements;
				uint32 useable;

				if (!Event::Read(event, this->expected - this->received, &elements, &useable, yield))
					return;

				this->received += useable;

				if (this->received == this->expected)
				{
					switch_to_state(State::done_state);
				}
				else
				{
					(*yield) = true;
				}
			}
			break;
		
		default:
			throw new Exception("Basic::IgnoreFrame::Process unexpected state");
		}
	}
}