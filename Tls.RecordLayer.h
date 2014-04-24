// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Basic.ElementSource.h"
#include "Tls.SecurityParameters.h"
#include "Tls.RecordFrame.h"
#include "Tls.ConnectionState.h"
#include "Tls.AlertProtocol.h"
#include "Tls.ServerHandshake.h"
#include "Basic.IBufferedStream.h"
#include "Tls.ICertificate.h"
#include "Tls.HeartbeatProtocol.h"

namespace Tls
{
    using namespace Basic;

    class HandshakeProtocol;
    class ServerHandshake;
    class ClientHandshake;
    class RecordStream;

    class RecordLayer : public Frame, public IBufferedStream<byte>
    {
    private:
        enum State
        {
            unconnected_state = Start_State,
            receive_record_state,
            record_frame_pending_state,
            done_state = Succeeded_State,
            record_frame_failed,
            record_length_too_large_error,
            record_version_mismatch_error,
            application_stream_failed,
            change_cipher_spec_zero_fragment_error,
            handshake_protocol_failed,
            alert_zero_fragment_error,
            alert_protocol_failed,
            unexpected_heartbeat_error,
            heartbeat_zero_fragment_error,
            heartbeat_protocol_failed,
            handshake_zero_fragment_error,
            unexpected_application_data_error,
            unexpected_record_type_error,
            send_zero_fragment_error,
            compress_unexpected_compression_algorithm_error,
            encrypt_unexpected_cipher_type_error,
            encrypt_stream_output_overflow_error,
            encrypt_stream_unexpected_bulk_cipher_algorithm_error,
            encrypt_block_output_overflow_error,
            encrypt_block_unexpected_bulk_cipher_algorithm_error,
            decompress_unexpected_compression_algorithm_error,
            decrypt_unexpected_cipher_type_error,
            decrypt_stream_decryption_failed,
            decrypt_stream_mac_mismatch_error,
            decrypt_stream_unexpected_bulk_cipher_algorithm_error,
            decrypt_block_decryption_failed,
            decrypt_block_padding_overflow_error,
            decrypt_block_padding_invalid_error,
            decrypt_block_mac_mismatch_error,
            decrypt_block_unexpected_bulk_cipher_algorithm_error,
            version_already_finalized_error,
        };

        Inline<ElementSource<byte> > application_element_source;
        Basic::Ref<IProcess> application_stream; // REF

        Inline<ElementSource<byte> > alert_element_source;
        AlertProtocol::Ref alert_protocol; // REF

        Inline<ElementSource<byte> > handshake_element_source;
        HandshakeProtocol::Ref handshake_protocol; // REF

        Inline<ElementSource<byte> > heartbeat_element_source;
        HeartbeatProtocol::Ref heartbeat_protocol; // REF

        Record record;
        std::vector<opaque> session_id;
        ProtocolVersion version_low;
        ProtocolVersion version_high;
        ProtocolVersion version;
        bool version_finalized;
        ByteVector::Ref record_buffer; // REF
        ContentType buffer_type;
        ContentType current_type;
        Basic::Ref<IBufferedStream<byte> > transport_peer; // REF
        Inline<RecordFrame> record_frame;
        bool server;
        Basic::Ref<ICertificate> certificate;
        bool send_heartbeats;
        bool receive_heartbeats;
        bool application_connected;
        //bool handshake_in_progress; // $$ set and use for error checking per RFC 4346
        //bool heartbeat_in_flight; // $$ set and use for error checking per RFC 6520
        Inline<ByteVector> transport_buffer;

        ConnectionState::Ref pending_read_state; // REF
        ConnectionState::Ref pending_write_state; // REF
        ConnectionState::Ref active_read_state; // REF
        ConnectionState::Ref active_write_state; // REF

        void ConnectApplication();
        void DisconnectApplication();
        void FinalizeVersion(ProtocolVersion version);
        void ProcessRecord(Record* record);
        void WriteChangeCipherSpec();
        void Write(ContentType type, const byte* elements, uint32 count);

        void Compress(Record* plaintext, Record* compressed);
        void Encrypt(Record* compressed, Record* encrypted);
        void EncryptStream(Record* compressed, Record* encrypted);
        void EncryptBlock(Record* compressed, Record* encrypted);

        void Decompress(Record* compressed, Record* plaintext);
        void Decrypt(Record* encrypted, Record* compressed);
        void DecryptStream(Record* encrypted, Record* compressed);
        void DecryptBlock(Record* encrypted, Record* compressed);
        void FlushRecordBuffer();
        void CloseTransport();
        void WriteAlert(AlertDescription description, AlertLevel level);

        void switch_to_state(uint32 state);

    public:
        friend class Tls::HandshakeProtocol;
        friend class Tls::ServerHandshake;
        friend class Tls::ClientHandshake;
        friend class Tls::RecordStream;

        typedef Basic::Ref<RecordLayer, IProcess> Ref;

        void Initialize(IBufferedStream<byte>* peer, IProcess* application_stream, bool server, Basic::Ref<ICertificate> certificate);

        virtual void IProcess::Process(IEvent* event, bool* yield);

        virtual void IBufferedStream<byte>::Write(const byte* elements, uint32 count);
        virtual void IBufferedStream<byte>::WriteEOF();
        virtual void IBufferedStream<byte>::Flush();
    };
}