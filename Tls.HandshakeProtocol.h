// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Basic.MemoryRange.h"
#include "Tls.SecurityParameters.h"
#include "Tls.Types.h"
#include "Tls.HandshakeFrame.h"

namespace Tls
{
    using namespace Basic;

    class RecordLayer;

    class HandshakeProtocol : public Frame
    {
    protected:
        RecordLayer* session;
        std::shared_ptr<ByteString> handshake_messages;
        Handshake handshake;
        std::shared_ptr<SecurityParameters> security_parameters;
        HandshakeFrame handshake_frame;
        KeyExchangeAlgorithm key_exchange_algorithm;
        EncryptedPreMasterSecret pre_master_secret_bytes;
        std::shared_ptr<ByteString> finished_sent;
        ByteString finished_expected;
        ByteString finished_received;
        MemoryRange finished_received_frame;

        bool WriteMessage(IStream<byte>* stream, HandshakeType msg_type, IStreamWriter<byte>* message);
        bool WriteFinished(ByteString* label);
        void CalculateVerifyData(ByteString* label, byte* output, uint16 output_max);
        void CalculateKeys(IVector<byte>* pre_master_key);

        virtual void PartitionKeyMaterial(ByteString* key_material) = 0;
        void switch_to_state(uint32 state);
        void Complete(bool success);

    public:
        HandshakeProtocol(RecordLayer* session);
    };
}