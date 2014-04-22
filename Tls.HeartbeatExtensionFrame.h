// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Tls.Types.h"
#include "Tls.NumberFrame.h"

namespace Tls
{
	using namespace Basic;

	class HeartbeatExtensionFrame : public Frame, public ISerializable
	{
	private:
		enum State
		{
			start_state = Start_State,
			mode_frame_pending_state,
			done_state = Succeeded_State,
			mode_frame_failed,
		};

		HeartbeatExtension* heartbeat_extension;
		Inline<NumberFrame<HeartbeatMode> > mode_frame;

	public:
		typedef Basic::Ref<HeartbeatExtensionFrame, IProcess> Ref;

		void Initialize(HeartbeatExtension* heartbeat_extension);

		virtual void IProcess::Process(IEvent* event, bool* yield);
		virtual void ISerializable::SerializeTo(IStream<byte>* stream);
	};
}