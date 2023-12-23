// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.HeartbeatMessageFrame.h"

namespace Tls
{
    using namespace Basic;

    HeartbeatMessageFrame::HeartbeatMessageFrame(HeartbeatMessage* heartbeat_message) :
        heartbeat_message(heartbeat_message),
        type_frame(&this->heartbeat_message->type), // initialization is in order of declaration in class def
        payload_length_frame(&this->heartbeat_message->payload_length), // initialization is in order of declaration in class def
        plaintext_length(0)
    {
    }

    void HeartbeatMessageFrame::set_plaintext_length(uint32 plaintext_length)
    {
        this->plaintext_length = plaintext_length;
    }

	ConsumeElementsResult HeartbeatMessageFrame::consume_elements(IElementSource<byte>* element_source)
    {
        switch (this->get_state())
        {
        case State::type_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->type_frame, element_source, this, State::type_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::payload_length_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::payload_length_frame_pending_state:
        {
			ConsumeElementsResult result = Basic::consume_elements(&this->payload_length_frame, element_source, this, State::payload_length_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

            uint32 non_padding_length = this->heartbeat_message->payload_length + 3;

            // RFC 6520 section 4
            // If the payload_length of a received HeartbeatMessage is too large,
            // the received HeartbeatMessage MUST be discarded silently.
            if (non_padding_length > this->plaintext_length)
            {
                switch_to_state(State::payload_length_error);
				return ConsumeElementsResult::failed;
			}

            this->padding_length = this->plaintext_length - non_padding_length;

            // RFC 6520 section 4
            // The padding_length MUST be at least 16.
            if (this->padding_length < 16)
            {
                switch_to_state(State::payload_length_error);
				return ConsumeElementsResult::failed;
			}

            // RFC 6520 section 4
            // The total length of a HeartbeatMessage MUST NOT exceed 2^14 or
            // max_fragment_length when negotiated as defined in [RFC6066].
            if (this->plaintext_length > 0x4000)
            {
                switch_to_state(State::payload_length_error);
				return ConsumeElementsResult::failed;
			}

            this->heartbeat_message->payload.resize(this->heartbeat_message->payload_length);
            this->payload_frame.reset(this->heartbeat_message->payload.address(), this->heartbeat_message->payload.size());

            switch_to_state(State::payload_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::payload_frame_pending_state:
        {
			ConsumeElementsResult result = Basic::consume_elements(&this->payload_frame, element_source, this, State::payload_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

            this->heartbeat_message->padding.resize(this->padding_length);
            this->padding_frame.reset(this->heartbeat_message->padding.address(), this->heartbeat_message->padding.size());

            switch_to_state(State::padding_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::padding_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->padding_frame, element_source, this, State::padding_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::done_state);
			return ConsumeElementsResult::succeeded;
		}

        default:
            throw FatalError("HeartbeatMessageFrame::handle_event unexpected state");
        }
    }
}