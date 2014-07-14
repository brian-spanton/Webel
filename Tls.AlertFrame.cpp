// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.AlertFrame.h"

namespace Tls
{
    using namespace Basic;

    AlertFrame::AlertFrame(Alert* alert) :
        alert(alert),
        level_frame(&this->alert->level),
        description_frame(&this->alert->description)
    {
    }

    void AlertFrame::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::level_frame_pending_state:
            delegate_event_change_state_on_fail(&this->level_frame, event, State::level_frame_failed);
            switch_to_state(State::description_frame_pending_state);
            break;

        case State::description_frame_pending_state:
            delegate_event_change_state_on_fail(&this->description_frame, event, State::description_frame_failed);
            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("Tls::AlertFrame::handle_event unexpected state");
        }
    }
}
