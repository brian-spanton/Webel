// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Basic.MemoryRange.h"
#include "Tls.SecurityParameters.h"
#include "Tls.Types.h"
#include "Tls.HandshakeFrame.h"
#include "Basic.ByteVector.h"

namespace Tls
{
    using namespace Basic;

    class RecordLayer;

    class HandshakeProtocol : public Frame
    {
    protected:
        RecordLayer* session;
        ByteVector::Ref handshake_messages; // REF
        Handshake handshake;
        SecurityParameters::Ref security_parameters; // REF
        Inline<HandshakeFrame> handshake_frame;
        KeyExchangeAlgorithm key_exchange_algorithm;
        ByteVector::Ref pre_master_secret_bytes; // REF
        ByteVector::Ref finished_sent; // REF
        std::vector<opaque> finished_expected;
        std::vector<opaque> finished_received;
        Inline<MemoryRange> finished_received_frame;

        bool WriteMessage(IStream<byte>* stream, HandshakeType msg_type, ISerializable* message);
        bool WriteFinished(opaque* label, uint16 label_length);
        void CalculateVerifyData(opaque* label, uint16 label_length, opaque* output, uint16 output_max);
        void CalculateKeys(ByteVector* pre_master_key);

        virtual void PartitionKeyMaterial(std::vector<opaque>* key_material) = 0;
        void switch_to_state(uint32 state);
        void Complete(bool success);

    public:
        typedef Basic::Ref<HandshakeProtocol, IProcess> Ref;

        void Initialize(RecordLayer* session);

        virtual void IProcess::Process(IEvent* event, bool* yield) = 0;
    };
}