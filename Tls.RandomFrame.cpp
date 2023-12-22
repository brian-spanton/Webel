// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.RandomFrame.h"

namespace Tls
{
    using namespace Basic;

    RandomFrame::RandomFrame(Random* random) :
        random(random),
        time_frame(&this->random->gmt_unix_time), // initialization is in order of declaration in class def
        bytes_frame(this->random->random_bytes, sizeof(this->random->random_bytes)) // initialization is in order of declaration in class def
    {
    }

    EventResult RandomFrame::consider_event(IEvent* event)
    {
        EventResult result;

        switch (get_state())
        {
        case State::time_frame_pending_state:
            result = delegate_event_change_state_on_fail(&this->time_frame, event, State::time_frame_failed);
            if (result == event_result_yield)
                return event_result_yield;

            switch_to_state(State::bytes_frame_pending_state);
            break;

        case State::bytes_frame_pending_state:
            result = delegate_event_change_state_on_fail(&this->bytes_frame, event, State::bytes_frame_failed);
            if (result == event_result_yield)
                return event_result_yield;

            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("Tls::RandomFrame unexpected state");
        }

        return event_result_continue;
    }
}
