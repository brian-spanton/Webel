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

    ProcessResult HeartbeatExtensionFrame::process_event(IEvent* event)
    {
        ProcessResult result;

        switch (get_state())
        {
        case State::mode_frame_pending_state:
            result = process_event_change_state_on_fail(&this->mode_frame, event, State::mode_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("HeartbeatExtensionFrame::handle_event unexpected state");
        }

        return ProcessResult::process_result_ready;
    }
}