// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.ServerNameFrame.h"
#include "Tls.Globals.h"

namespace Tls
{
    using namespace Basic;

    ServerNameFrame::ServerNameFrame(ServerName* serverName) :
        serverName(serverName),
        type_frame(&this->serverName->name_type), // initialization is in order of declaration in class def
        name_frame(&this->serverName->name) // initialization is in order of declaration in class def
    {
    }

    ProcessResult ServerNameFrame::consider_event(IEvent* event)
    {
        ProcessResult result;

        switch (get_state())
        {
        case State::start_state:
            switch_to_state(State::type_state);
            break;

        case State::type_state:
            result = delegate_event_change_state_on_fail(&this->type_frame, event, State::type_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::name_state);
            break;

        case State::name_state:
            result = delegate_event_change_state_on_fail(&this->name_frame, event, State::name_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("Tls::ServerNameFrame::handle_event unexpected state");
        }

        return ProcessResult::process_result_ready;
    }
}