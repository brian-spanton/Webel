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

    EventResult CertificateStatusRequestFrame::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::type_frame_pending_state:
            {
                EventResult result = delegate_event_change_state_on_fail(&this->type_frame, event, State::type_frame_failed);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

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
                EventResult result = delegate_event_change_state_on_fail(&this->request_frame, event, State::request_frame_failed);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                switch_to_state(State::done_state);
            }
            break;

        default:
            throw FatalError("CertificateStatusRequestFrame::handle_event unexpected state");
        }

        return EventResult::event_result_continue;
    }
}