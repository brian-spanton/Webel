// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Tls.Types.h"
#include "Tls.AlertFrame.h"

namespace Tls
{
	using namespace Basic;

	class RecordLayer;

	class AlertProtocol : public Frame
	{
	protected:
		enum State
		{
			alert_frame_pending_state = Start_State,
			done_state = Succeeded_State,
		};

		RecordLayer* session;
		Alert alert;
		Inline<AlertFrame> alert_frame;

	public:
		typedef Basic::Ref<AlertProtocol, IProcess> Ref;

		void Initialize(RecordLayer* session);
		virtual void IProcess::Process(IEvent* event, bool* yield);
	};
}