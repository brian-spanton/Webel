// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Tls.Types.h"
#include "Tls.NumberFrame.h"
#include "Tls.VectorFrames.h"

namespace Tls
{
	using namespace Basic;

	class HeartbeatMessageFrame : public Frame, public ISerializable
	{
	private:
		enum State
		{
			start_state = Start_State,
			type_frame_pending_state,
			payload_length_frame_pending_state,
			payload_frame_pending_state,
			padding_frame_pending_state,
			done_state = Succeeded_State,
			type_frame_failed,
			payload_length_frame_failed,
			payload_length_error,
			payload_frame_failed,
			padding_frame_failed,
		};

		HeartbeatMessage* heartbeat_message;
		uint32 plaintext_length;
		uint32 padding_length;
		Inline<NumberFrame<HeartbeatMessageType> > type_frame;
		Inline<NumberFrame<uint16> > payload_length_frame;
		Inline<MemoryRange> payload_frame;
		Inline<MemoryRange> padding_frame;

	public:
		typedef Basic::Ref<HeartbeatMessageFrame, IProcess> Ref;

		void Initialize(HeartbeatMessage* heartbeat_message, uint32 plaintext_length);

		virtual void IProcess::Process(IEvent* event, bool* yield);
		virtual void ISerializable::SerializeTo(IStream<byte>* stream);
	};
}