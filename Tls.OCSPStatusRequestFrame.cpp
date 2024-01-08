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

    ProcessResult OCSPStatusRequestFrame::process_event(IEvent* event)
    {
        ProcessResult result;

        switch (get_state())
        {
        case State::list_frame_pending_state:
            result = process_event_change_state_on_fail(&this->list_frame, event, State::list_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::extensions_frame_pending_state);
            break;

        case State::extensions_frame_pending_state:
            result = process_event_change_state_on_fail(&this->extensions_frame, event, State::extensions_frame_failed);
            if (result == process_result_blocked)
                return ProcessResult::process_result_blocked;

            switch_to_state(State::done_state);
            break;

        default:
            throw FatalError("Tls", "OCSPStatusRequestFrame::process_event unhandled state");
        }

        return ProcessResult::process_result_ready;
    }
}