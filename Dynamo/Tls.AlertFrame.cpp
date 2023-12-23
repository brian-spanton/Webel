#include "stdafx.h"
#include "Tls.AlertFrame.h"

namespace Tls
{
	using namespace Basic;

	void AlertFrame::Initialize(Alert* alert)
	{
		__super::Initialize();

		this->alert = alert;

		this->level_frame.Initialize(&this->alert->level);
		this->description_frame.Initialize(&this->alert->description);
	}

	void AlertFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::level_frame_pending_state:
			if (this->level_frame.Pending())
			{
				this->level_frame.Process(event, yield);
			}
			else if (this->level_frame.Failed())
			{
				throw new Exception("level_frame.Failed()");
			}
			else
			{
				switch_to_state(State::description_frame_pending_state);
			}
			break;

		case State::description_frame_pending_state:
			if (this->description_frame.Pending())
			{
				this->description_frame.Process(event, yield);
			}
			else if (this->description_frame.Failed())
			{
				throw new Exception("description_frame.Failed()");
			}
			else
			{
				switch_to_state(State::done_state);
			}
			break;

		default:
			throw new Exception("Tls::AlertFrame::Process unexpected state");
		}
	}

	void AlertFrame::SerializeTo(IStream<byte>* stream)
	{
		this->level_frame.SerializeTo(stream);
		this->description_frame.SerializeTo(stream);
	}
}
