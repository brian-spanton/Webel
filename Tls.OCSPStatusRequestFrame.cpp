// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.OCSPStatusRequestFrame.h"

namespace Tls
{
    using namespace Basic;

    OCSPStatusRequestFrame::OCSPStatusRequestFrame(OCSPStatusRequest* ocsp_status_request) :
        ocsp_status_request(ocsp_status_request),
        list_frame(&this->ocsp_status_request->responder_id_list), // order of declaration is important
        extensions_frame(&this->ocsp_status_request->request_extensions) // order of declaration is important
    {
    }

    void OCSPStatusRequestFrame::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::list_frame_pending_state:
            delegate_event_change_state_on_fail(&this->list_frame, event, State::list_frame_failed);
            switch_to_state(State::extensions_frame_pending_state);
            break;

        case State::extensions_frame_pending_state:
            delegate_event_change_state_on_fail(&this->extensions_frame, event, State::extensions_frame_failed);
            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("OCSPStatusRequestFrame::handle_event unexpected state");
        }
    }
}