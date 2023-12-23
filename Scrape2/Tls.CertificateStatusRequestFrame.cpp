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

	ConsumeElementsResult CertificateStatusRequestFrame::consume_elements(IElementSource<byte>* element_source)
    {
        switch (this->get_state())
        {
        case State::type_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->type_frame, element_source, this, State::type_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			switch (this->certificate_status_request->status_type)
			{
			case CertificateStatusType::ocsp:
				switch_to_state(State::request_frame_pending_state);
				return ConsumeElementsResult::in_progress;

			default:
				switch_to_state(State::type_state_error);
				return ConsumeElementsResult::failed;
			}
		}

        case State::request_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->request_frame, element_source, this, State::request_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::done_state);
			return ConsumeElementsResult::succeeded;
		}

        default:
            throw FatalError("CertificateStatusRequestFrame::handle_event unexpected state");
        }
    }
}