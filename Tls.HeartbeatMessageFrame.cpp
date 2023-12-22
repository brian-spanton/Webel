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

    EventResult HeartbeatMessageFrame::consider_event(IEvent* event)
    {
        EventResult result;

        switch (get_state())
        {
        case State::type_frame_pending_state:
            result = delegate_event_change_state_on_fail(&this->type_frame, event, State::type_frame_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::payload_length_frame_pending_state);
            break;

        case State::payload_length_frame_pending_state:
            {
                result = delegate_event_change_state_on_fail(&this->payload_length_frame, event, State::payload_length_frame_failed);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                uint32 non_padding_length = this->heartbeat_message->payload_length + 3;

                // RFC6520 section 4
                // If the payload_length of a received HeartbeatMessage is too large,
                // the received HeartbeatMessage MUST be discarded silently.
                if (non_padding_length > this->plaintext_length)
                {
                    switch_to_state(State::payload_length_error);
                    return EventResult::event_result_continue;
                }

                this->padding_length = this->plaintext_length - non_padding_length;

                // RFC6520 section 4
                // The padding_length MUST be at least 16.
                if (this->padding_length < 16)
                {
                    switch_to_state(State::payload_length_error);
                    return EventResult::event_result_continue;
                }

                // RFC6520 section 4
                // The total length of a HeartbeatMessage MUST NOT exceed 2^14 or
                // max_fragment_length when negotiated as defined in [RFC6066].
                if (this->plaintext_length > 0x4000)
                {
                    switch_to_state(State::payload_length_error);
                    return EventResult::event_result_continue;
                }

                this->heartbeat_message->payload.resize(this->heartbeat_message->payload_length);
                this->payload_frame.reset(this->heartbeat_message->payload.address(), this->heartbeat_message->payload.size());
                switch_to_state(State::payload_frame_pending_state);
            }
            break;

        case State::payload_frame_pending_state:
            {
                result = delegate_event_change_state_on_fail(&this->payload_frame, event, State::payload_frame_failed);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                this->heartbeat_message->padding.resize(this->padding_length);
                this->padding_frame.reset(this->heartbeat_message->padding.address(), this->heartbeat_message->padding.size());
                switch_to_state(State::padding_frame_pending_state);
            }
            break;

        case State::padding_frame_pending_state:
            result = delegate_event_change_state_on_fail(&this->padding_frame, event, State::padding_frame_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("HeartbeatMessageFrame::handle_event unexpected state");
        }

        return EventResult::event_result_continue;
    }
}