// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.RandomFrame.h"

namespace Tls
{
    using namespace Basic;

    RandomFrame::RandomFrame(Random* random) :
        random(random),
        time_frame(&this->random->gmt_unix_time), // order of declaration is important
        bytes_frame(this->random->random_bytes, sizeof(this->random->random_bytes)) // order of declaration is important
    {
    }

    void RandomFrame::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::time_frame_pending_state:
            {
                delegate_event_change_state_on_fail(&this->time_frame, event, State::time_frame_failed);

                switch_to_state(State::bytes_frame_pending_state);
            }
            break;

        case State::bytes_frame_pending_state:
            {
                delegate_event_change_state_on_fail(&this->bytes_frame, event, State::bytes_frame_failed);

                switch_to_state(State::done_state);
            }
            break;

        default:
            throw FatalError("Tls::RandomFrame unexpected state");
        }
    }
}
