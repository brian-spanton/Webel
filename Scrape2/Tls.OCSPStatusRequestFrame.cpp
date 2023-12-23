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

	ConsumeElementsResult OCSPStatusRequestFrame::consume_elements(IElementSource<byte>* element_source)
    {
        switch (this->get_state())
        {
        case State::list_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->list_frame, element_source, this, State::list_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::extensions_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::extensions_frame_pending_state:
		{
			ConsumeElementsResult result = Basic::consume_elements(&this->extensions_frame, element_source, this, State::extensions_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

			this->switch_to_state(State::done_state);
			return ConsumeElementsResult::succeeded;
		}

        default:
            throw FatalError("OCSPStatusRequestFrame::handle_event unexpected state");
        }
    }
}