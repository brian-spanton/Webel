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

    EventResult ServerNameFrame::consider_event(IEvent* event)
    {
        EventResult result;

        switch (get_state())
        {
        case State::start_state:
            switch_to_state(State::type_state);
            break;

        case State::type_state:
            result = delegate_event_change_state_on_fail(&this->type_frame, event, State::type_frame_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::name_state);
            break;

        case State::name_state:
            result = delegate_event_change_state_on_fail(&this->name_frame, event, State::name_frame_failed);
            if (result == event_result_yield)
                return EventResult::event_result_yield;

            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("Tls::ServerNameFrame::handle_event unexpected state");
        }

        return EventResult::event_result_continue;
    }
}