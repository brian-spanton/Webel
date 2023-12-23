// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.MemoryRange.h"
#include "Tls.SecurityParameters.h"
#include "Tls.Types.h"
#include "Tls.HandshakeFrame.h"
#include "Basic.ITransportEventHandler.h"

namespace Tls
{
    using namespace Basic;

    class RecordLayer;

    class HandshakeProtocol : public StateMachine, public ITransportEventHandler<byte>, public IElementConsumer<byte>
    {
    protected:
        RecordLayer* session;
        ByteStringRef send_buffer;
        std::shared_ptr<ByteString> handshake_messages;
        Handshake handshake;
        std::shared_ptr<SecurityParameters> security_parameters;
        HandshakeFrame handshake_frame;
        KeyExchangeAlgorithm key_exchange_algorithm;
        EncryptedPreMasterSecret pre_master_secret_bytes;
        ByteString finished_expected;
        ByteString finished_received;
        MemoryRange finished_received_frame;

        void WriteMessage(HandshakeType msg_type, IStreamWriter<byte>* message);
        void WriteFinished(ByteString* label);
        void CalculateVerifyData(ByteString* label, byte* output, uint16 output_max);
        void CalculateKeys(IVector<byte>* pre_master_key);

        virtual void PartitionKeyMaterial(ByteString* key_material) = 0;
        void switch_to_state(uint32 state);
        void Complete(bool success);

    public:
        HandshakeProtocol(RecordLayer* session);

		void ITransportEventHandler<byte>::transport_received(const byte* elements, uint32 count);

		virtual void change_cipher_spec_event() = 0;
	};
}