// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.AlertFrame.h"

namespace Tls
{
    using namespace Basic;

    AlertFrame::AlertFrame(Alert* alert) :
        alert(alert),
        level_frame(&this->alert->level), // initialization is in order of declaration in class def
        description_frame(&this->alert->description) // initialization is in order of declaration in class def
    {
    }

    event_result AlertFrame::consider_event(IEvent* event)
    {
        event_result result;

        switch (get_state())
        {
        case State::level_frame_pending_state:
            result = delegate_event_change_state_on_fail(&this->level_frame, event, State::level_frame_failed);
            if (result == event_result_yield)
                return event_result_yield;

            switch_to_state(State::description_frame_pending_state);
            break;

        case State::description_frame_pending_state:
            result = delegate_event_change_state_on_fail(&this->description_frame, event, State::description_frame_failed);
            if (result == event_result_yield)
                return event_result_yield;

            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("Tls::AlertFrame::handle_event unexpected state");
        }

        return event_result_continue;
    }
}
