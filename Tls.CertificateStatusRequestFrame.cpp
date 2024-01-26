// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.CertificateStatusRequestFrame.h"

namespace Tls
{
    using namespace Basic;

    CertificateStatusRequestFrame::CertificateStatusRequestFrame(CertificateStatusRequest* certificate_status_request) :
        certificate_status_request(certificate_status_request),
        type_frame(&this->certificate_status_request->status_type), // initialization is in order of declaration in class def
        request_frame(&this->certificate_status_request->ocsp_status_request) // initialization is in order of declaration in class def
    {
    }

    ProcessResult CertificateStatusRequestFrame::process_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::type_frame_pending_state:
            {
                ProcessResult result = process_event_change_state_on_fail(&this->type_frame, event, State::type_frame_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                switch (this->certificate_status_request->status_type)
                {
                case CertificateStatusType::ocsp:
                    switch_to_state(State::request_frame_pending_state);
                    break;

                default:
                    switch_to_state(State::type_state_error);
                    break;
                }
            }
            break;

        case State::request_frame_pending_state:
            {
                ProcessResult result = process_event_change_state_on_fail(&this->request_frame, event, State::request_frame_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                switch_to_state(State::done_state);
            }
            break;

        default:
            throw FatalError("Tls", "CertificateStatusRequestFrame", "process_event", "unhandled state", this->get_state());
        }

        return ProcessResult::process_result_ready;
    }
}