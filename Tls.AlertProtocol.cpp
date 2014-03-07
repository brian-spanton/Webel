// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.AlertProtocol.h"

#include "Tls.RecordLayer.h"

namespace Tls
{
	using namespace Basic;

	void AlertProtocol::Initialize(RecordLayer* session)
	{
		__super::Initialize();
		this->session = session;
		this->alert_frame.Initialize(&this->alert);
	}

	void AlertProtocol::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::alert_frame_pending_state:
			if (this->alert_frame.Pending())
			{
				this->alert_frame.Process(event, yield);
			}
			else if (this->alert_frame.Failed())
			{
				throw new Exception("alert_frame.Failed()");
			}
			else
			{
				this->alert_frame.Initialize(&this->alert);
			}
			break;

		default:
			throw new Exception("Tls::AlertProtocol::Process unexpected state");
		}
	}
}
