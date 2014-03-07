// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.CertificateStatusRequestFrame.h"

namespace Tls
{
	using namespace Basic;

	void CertificateStatusRequestFrame::Initialize(CertificateStatusRequest* certificate_status_request)
	{
		__super::Initialize();
		this->certificate_status_request = certificate_status_request;
		this->type_frame.Initialize(&this->certificate_status_request->status_type);
		this->request_frame.Initialize(&this->certificate_status_request->ocsp_status_request);
	}

	void CertificateStatusRequestFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::type_frame_pending_state:
			if (this->type_frame.Pending())
			{
				this->type_frame.Process(event, yield);
			}
			else if (this->type_frame.Failed())
			{
				switch_to_state(State::type_frame_failed);
			}
			else
			{
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
			if (this->request_frame.Pending())
			{
				this->request_frame.Process(event, yield);
			}
			else if (this->request_frame.Failed())
			{
				switch_to_state(State::request_frame_failed);
			}
			else
			{
				switch_to_state(State::done_state);
			}
			break;

		default:
			throw new Exception("CertificateStatusRequestFrame::Process unexpected state");
		}
	}
}