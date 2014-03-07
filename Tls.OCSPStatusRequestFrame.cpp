// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.OCSPStatusRequestFrame.h"
#include "Tls.ResponderIDListFrame.h"

namespace Tls
{
	using namespace Basic;

	void OCSPStatusRequestFrame::Initialize(OCSPStatusRequest* ocsp_status_request)
	{
		__super::Initialize();
		this->ocsp_status_request = ocsp_status_request;
		this->list_frame.Initialize(&this->ocsp_status_request->responder_id_list);
		this->extensions_frame.Initialize(&this->ocsp_status_request->request_extensions);
	}

	void OCSPStatusRequestFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::list_frame_pending_state:
			if (this->list_frame.Pending())
			{
				this->list_frame.Process(event, yield);
			}
			else if (this->list_frame.Failed())
			{
				switch_to_state(State::list_frame_failed);
			}
			else
			{
				switch_to_state(State::extensions_frame_pending_state);
			}
			break;

		case State::extensions_frame_pending_state:
			if (this->extensions_frame.Pending())
			{
				this->extensions_frame.Process(event, yield);
			}
			else if (this->extensions_frame.Failed())
			{
				switch_to_state(State::extensions_frame_failed);
			}
			else
			{
				switch_to_state(State::done_state);
			}
			break;

		default:
			throw new Exception("OCSPStatusRequestFrame::Process unexpected state");
		}
	}
}