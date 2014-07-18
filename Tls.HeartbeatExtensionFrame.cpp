// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.HeartbeatExtensionFrame.h"

namespace Tls
{
    using namespace Basic;

    HeartbeatExtensionFrame::HeartbeatExtensionFrame(HeartbeatExtension* heartbeat_extension) :
        heartbeat_extension(heartbeat_extension),
        mode_frame(&this->heartbeat_extension->mode) // order of declaration is important
    {
    }

    void HeartbeatExtensionFrame::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::mode_frame_pending_state:
            delegate_event_change_state_on_fail(&this->mode_frame, event, State::mode_frame_failed);
            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("HeartbeatExtensionFrame::handle_event unexpected state");
        }
    }
}