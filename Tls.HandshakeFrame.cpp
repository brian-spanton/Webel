// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.HandshakeFrame.h"

namespace Tls
{
	using namespace Basic;

	void HandshakeFrame::Initialize(Handshake* handshake)
	{
		__super::Initialize();
		this->handshake = handshake;
		this->type_frame.Initialize(&this->handshake->msg_type);
		this->length_frame.Initialize(&this->handshake->length);
	}

	void HandshakeFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::type_frame_pending_state:
			if (this->type_frame.Pending())
			{
				this->type_frame.Process(event, yield);
			}
			else if (this->type_frame.Failed())
			{
				switch_to_state(State::type_frame_failed);
			}
			else
			{
				switch_to_state(State::length_frame_pending_state);
			}
			break;

		case State::length_frame_pending_state:
			if (this->length_frame.Pending())
			{
				this->length_frame.Process(event, yield);
			}
			else if (this->length_frame.Failed())
			{
				switch_to_state(State::length_frame_failed);
			}
			else
			{
				switch_to_state(State::done_state);
			}
			break;

		default:
			throw new Exception("Tls::HandshakeFrame::Process unexpected state");
		}
	}

	void HandshakeFrame::SerializeTo(IStream<byte>* stream)
	{
		this->type_frame.SerializeTo(stream);
		this->length_frame.SerializeTo(stream);
	}
}
