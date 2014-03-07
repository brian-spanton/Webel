// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.RandomFrame.h"

namespace Tls
{
	using namespace Basic;

	void RandomFrame::Initialize(Random* random)
	{
		__super::Initialize();
		this->random = random;
		this->time_frame.Initialize(&this->random->gmt_unix_time);
		this->bytes_frame.Initialize(this->random->random_bytes, sizeof(this->random->random_bytes));
	}

	void RandomFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::time_frame_pending_state:
			if (this->time_frame.Pending())
			{
				this->time_frame.Process(event, yield);
			}
			else if (this->time_frame.Failed())
			{
				switch_to_state(State::time_frame_failed);
			}
			else
			{
				switch_to_state(State::bytes_frame_pending_state);
			}
			break;

		case State::bytes_frame_pending_state:
			if (this->bytes_frame.Pending())
			{
				this->bytes_frame.Process(event, yield);
			}
			else if (this->bytes_frame.Failed())
			{
				switch_to_state(State::bytes_frame_failed);
			}
			else
			{
				switch_to_state(State::done_state);
			}
			break;

		default:
			throw new Exception("Tls::RandomFrame unexpected state");
		}
	}

	void RandomFrame::SerializeTo(IStream<byte>* stream)
	{
		time_frame.SerializeTo(stream);
		bytes_frame.SerializeTo(stream);
	}
}
