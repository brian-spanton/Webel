// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Tls.Types.h"
#include "Tls.HeartbeatMessageFrame.h"

namespace Tls
{
	using namespace Basic;

	class RecordLayer;

	class HeartbeatProtocol : public Frame
	{
	protected:
		enum State
		{
			start_state = Start_State,
			heartbeat_message_frame_pending_state,
			done_state = Succeeded_State,
			heartbeat_message_frame_failed,
			unexpected_type_error,
		};

		RecordLayer* session;
		HeartbeatMessage heartbeat_message;
		Inline<HeartbeatMessageFrame> heartbeat_message_frame;

	public:
		typedef Basic::Ref<HeartbeatProtocol, IProcess> Ref;

		void Initialize(RecordLayer* session);
		virtual void IProcess::Process(IEvent* event, bool* yield);
		void SetPlaintextLength(uint16 plaintext_length);
	};
}