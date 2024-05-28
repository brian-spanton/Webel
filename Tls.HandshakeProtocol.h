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
        RecordLayer* session = 0;
        ByteStringRef send_buffer;
        std::shared_ptr<ByteString> handshake_messages;
        Handshake handshake;
        std::shared_ptr<SecurityParameters> security_parameters;
        HandshakeFrame handshake_frame;
        KeyExchangeAlgorithm key_exchange_algorithm = DHE_DSS;
        EncryptedPreMasterSecret pre_master_secret_bytes;
        ByteString finished_expected;
        ByteString finished_received;
        MemoryRange finished_received_frame;

        bool WriteMessage(HandshakeType msg_type, IStreamWriter<byte>* message);
        bool WriteFinished(ByteString* label);
        void CalculateVerifyData(ByteString* label, ByteString* output);
        void CalculateKeys(IVector<byte>* pre_master_key);

        virtual void PartitionKeyMaterial(IStreamWriter<byte>** keyExpansionSeed, uint32 keyExpansionSeedCount) = 0;
        void GenerateKeyMaterial(IStreamWriter<byte>** keyExpansionSeed, uint32 keyExpansionSeedCount, byte* output, uint32 output_length);
        void GenerateRandom(Random* random);
        void switch_to_state(uint32 state);
        void Complete(bool success);

    public:
        HandshakeProtocol(RecordLayer* session);
    };
}