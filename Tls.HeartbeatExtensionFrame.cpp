// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.HeartbeatExtensionFrame.h"

namespace Tls
{
    using namespace Basic;

    HeartbeatExtensionFrame::HeartbeatExtensionFrame(HeartbeatExtension* heartbeat_extension) :
        heartbeat_extension(heartbeat_extension),
        mode_frame(&this->heartbeat_extension->mode) // initialization is in order of declaration in class def
    {
    }

    event_result HeartbeatExtensionFrame::consider_event(IEvent* event)
    {
        event_result result;

        switch (get_state())
        {
        case State::mode_frame_pending_state:
            result = delegate_event_change_state_on_fail(&this->mode_frame, event, State::mode_frame_failed);
            if (result == event_result_yield)
                return event_result_yield;

            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("HeartbeatExtensionFrame::handle_event unexpected state");
        }

        return event_result_continue;
    }
}