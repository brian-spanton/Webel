// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Tls.Types.h"
#include "Tls.NumberFrame.h"
#include "Tls.OCSPStatusRequestFrame.h"

namespace Tls
{
	using namespace Basic;

	class CertificateStatusRequestFrame : public Frame
	{
	private:
		enum State
		{
			type_frame_pending_state = Start_State,
			request_frame_pending_state,
			done_state = Succeeded_State,
			type_frame_failed,
			type_state_error,
			request_frame_failed,
		};

		CertificateStatusRequest* certificate_status_request;
		Inline<NumberFrame<CertificateStatusType> > type_frame;
		Inline<OCSPStatusRequestFrame> request_frame;

	public:
		typedef Basic::Ref<CertificateStatusRequestFrame, IProcess> Ref;

		void Initialize(CertificateStatusRequest* certificate_status_request);
		virtual void IProcess::Process(IEvent* event, bool* yield);
	};
}