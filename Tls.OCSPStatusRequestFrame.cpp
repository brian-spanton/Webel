// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.OCSPStatusRequestFrame.h"

namespace Tls
{
    using namespace Basic;

    OCSPStatusRequestFrame::OCSPStatusRequestFrame(OCSPStatusRequest* ocsp_status_request) :
        ocsp_status_request(ocsp_status_request),
        list_frame(&this->ocsp_status_request->responder_id_list), // initialization is in order of declaration in class def
        extensions_frame(&this->ocsp_status_request->request_extensions) // initialization is in order of declaration in class def
    {
    }

    event_result OCSPStatusRequestFrame::consider_event(IEvent* event)
    {
        event_result result;

        switch (get_state())
        {
        case State::list_frame_pending_state:
            result = delegate_event_change_state_on_fail(&this->list_frame, event, State::list_frame_failed);
            if (result == event_result_yield)
                return event_result_yield;

            switch_to_state(State::extensions_frame_pending_state);
            break;

        case State::extensions_frame_pending_state:
            result = delegate_event_change_state_on_fail(&this->extensions_frame, event, State::extensions_frame_failed);
            if (result == event_result_yield)
                return event_result_yield;

            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("OCSPStatusRequestFrame::handle_event unexpected state");
        }

        return event_result_continue;
    }
}