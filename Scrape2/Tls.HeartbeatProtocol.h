// Copyright © 2013 Brian Spanton

#pragma once

#include "Tls.Types.h"
#include "Basic.StateMachine.h"
#include "Tls.HeartbeatMessageFrame.h"
#include "Basic.ITransportEventHandler.h"

namespace Tls
{
    using namespace Basic;

    class RecordLayer;

	class HeartbeatProtocol : public StateMachine, public ITransportEventHandler<byte>, public IElementConsumer<byte>
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
        HeartbeatMessageFrame heartbeat_message_frame;

    public:
        HeartbeatProtocol(RecordLayer* session);

        void SetPlaintextLength(uint16 plaintext_length);

		void ITransportEventHandler<byte>::transport_connected();
		void ITransportEventHandler<byte>::transport_disconnected();
		void ITransportEventHandler<byte>::transport_received(const byte* elements, uint32 count);

		ConsumeElementsResult IElementConsumer<byte>::consume_elements(IElementSource<byte>* element_source);
	};
}