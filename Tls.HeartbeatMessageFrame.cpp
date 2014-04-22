// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.HeartbeatMessageFrame.h"

namespace Tls
{
	using namespace Basic;

	void HeartbeatMessageFrame::Initialize(HeartbeatMessage* heartbeat_message, uint32 plaintext_length)
	{
		__super::Initialize();

		this->heartbeat_message = heartbeat_message;
		this->plaintext_length = plaintext_length;
	}

	void HeartbeatMessageFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::start_state:
			(*yield) = false;
			this->type_frame.Initialize(&this->heartbeat_message->type);
			switch_to_state(State::type_frame_pending_state);
			break;

		case State::type_frame_pending_state:
			if (this->type_frame.Pending())
			{
				this->type_frame.Process(event, yield);
			}

			if (this->type_frame.Failed())
			{
				switch_to_state(State::type_frame_failed);
			}
			else if (this->type_frame.Succeeded())
			{
				this->payload_length_frame.Initialize(&this->heartbeat_message->payload_length);
				switch_to_state(State::payload_length_frame_pending_state);
			}
			break;

		case State::payload_length_frame_pending_state:
			if (this->payload_length_frame.Pending())
			{
				this->payload_length_frame.Process(event, yield);
			}

			if (this->payload_length_frame.Failed())
			{
				switch_to_state(State::payload_length_frame_failed);
			}
			else if (this->payload_length_frame.Succeeded())
			{
				uint32 non_padding_length = this->heartbeat_message->payload_length + 3;

				// RFC 6520 section 4
				// If the payload_length of a received HeartbeatMessage is too large,
				// the received HeartbeatMessage MUST be discarded silently.
				if (non_padding_length > this->plaintext_length)
				{
					switch_to_state(State::payload_length_error);
					return;
				}

				this->padding_length = this->plaintext_length - non_padding_length;

				// RFC 6520 section 4
				// The padding_length MUST be at least 16.
				if (this->padding_length < 16)
				{
					switch_to_state(State::payload_length_error);
					return;
				}

				// RFC 6520 section 4
				// The total length of a HeartbeatMessage MUST NOT exceed 2^14 or
				// max_fragment_length when negotiated as defined in [RFC6066].
				if (this->plaintext_length > 0x4000)
				{
					switch_to_state(State::payload_length_error);
					return;
				}

				this->heartbeat_message->payload.resize(this->heartbeat_message->payload_length);
				this->payload_frame.Initialize(&this->heartbeat_message->payload[0], this->heartbeat_message->payload.size());
				switch_to_state(State::payload_frame_pending_state);
			}
			break;

		case State::payload_frame_pending_state:
			if (this->payload_frame.Pending())
			{
				this->payload_frame.Process(event, yield);
			}

			if (this->payload_frame.Failed())
			{
				switch_to_state(State::payload_frame_failed);
			}
			else if (this->payload_frame.Succeeded())
			{
				this->heartbeat_message->padding.resize(this->padding_length);
				this->padding_frame.Initialize(&this->heartbeat_message->padding[0], this->heartbeat_message->padding.size());
				switch_to_state(State::padding_frame_pending_state);
			}
			break;

		case State::padding_frame_pending_state:
			if (this->padding_frame.Pending())
			{
				this->padding_frame.Process(event, yield);
			}

			if (this->padding_frame.Failed())
			{
				switch_to_state(State::padding_frame_failed);
			}
			else if (this->padding_frame.Succeeded())
			{
				switch_to_state(State::done_state);
			}
			break;

		default:
			throw new Exception("HeartbeatMessageFrame::Process unexpected state");
		}
	}

	void HeartbeatMessageFrame::SerializeTo(IStream<byte>* stream)
	{
		this->type_frame.Initialize(&this->heartbeat_message->type);
		this->type_frame.SerializeTo(stream);

		this->payload_length_frame.Initialize(&this->heartbeat_message->payload_length);
		this->payload_length_frame.SerializeTo(stream);

		this->payload_frame.Initialize(&this->heartbeat_message->payload[0], this->heartbeat_message->payload.size());
		this->payload_frame.SerializeTo(stream);

		this->padding_frame.Initialize(&this->heartbeat_message->padding[0], this->heartbeat_message->payload.size());
		this->padding_frame.SerializeTo(stream);
	}
}