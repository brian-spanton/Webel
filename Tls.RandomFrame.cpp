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

    ProcessResult RandomFrame::process_event(IEvent* event)
    {
        ProcessResult result;

        switch (get_state())
        {
        case State::time_frame_pending_state:
            result = process_event_change_state_on_fail(&this->time_frame, event, State::time_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::bytes_frame_pending_state);
            break;

        case State::bytes_frame_pending_state:
            result = process_event_change_state_on_fail(&this->bytes_frame, event, State::bytes_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("Tls", "RandomFrame::process_event unhandled state");
        }

        return ProcessResult::process_result_ready;
    }
}
