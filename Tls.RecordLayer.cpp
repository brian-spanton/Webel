// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.RecordLayer.h"
#include "Tls.Globals.h"
#include "Tls.RandomFrame.h"
#include "Tls.ServerHandshake.h"
#include "Tls.ClientHandshake.h"
#include "Basic.Event.h"

namespace Tls
{
    using namespace Basic;

    void RecordLayer::Initialize(IBufferedStream<byte>* peer, IProcess* application_stream, bool server, Basic::Ref<ICertificate> certificate)
    {
        __super::Initialize();

        this->transport_peer = peer;
        this->application_stream = application_stream;
        this->application_connected = false;
        this->server = server;
        this->certificate = certificate;

        this->record_buffer = New<ByteVector>();
        this->record_buffer->reserve(0x400);

        this->version_low = 0x0301;
        this->version_high = 0x0302;
        this->version = this->version_high;
        this->version_finalized = false;

        this->session_id.resize(32);

        this->send_heartbeats = true; // $$
        this->receive_heartbeats = false;

        NTSTATUS error = BCryptGenRandom(0, &this->session_id[0], this->session_id.size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
        if (error != 0)
            throw new Exception("BCryptGenRandom", error);

        if (server)
        {
            ServerHandshake::Ref handshake_server = New<ServerHandshake>();
            handshake_server->Initialize(this);
            this->handshake_protocol = handshake_server;
        }
        else
        {
            ClientHandshake::Ref handshake_client = New<ClientHandshake>();
            handshake_client->Initialize(this);
            this->handshake_protocol = handshake_client;
        }

        this->alert_protocol = New<AlertProtocol>();
        this->alert_protocol->Initialize(this);

        this->heartbeat_protocol = New<HeartbeatProtocol>();
        this->heartbeat_protocol->Initialize(this);

        SecurityParameters::Ref security_parameters = New<SecurityParameters>();

        this->active_read_state = New<ConnectionState>();
        this->active_read_state->security_parameters = security_parameters;

        this->active_write_state = New<ConnectionState>();
        this->active_write_state->security_parameters = security_parameters;

        this->pending_read_state = New<ConnectionState>();
        this->pending_write_state = New<ConnectionState>();
    }

    void RecordLayer::Process(IEvent* event, bool* yield)
    {
        if (event->get_type() == Basic::EventType::element_stream_ending_event)
        {
            DisconnectApplication();
            (*yield) = true;
            return;
        }

        switch (frame_state())
        {
        case State::unconnected_state:
            if (event->get_type() != Basic::EventType::ready_for_write_bytes_event)
            {
                (*yield) = true;
            }
            else
            {
                this->current_type = ContentType::handshake;

                ReadyForWriteBytesEvent handshake_event;
                handshake_event.Initialize(&this->handshake_element_source);
                this->handshake_protocol->Frame::Process(&handshake_event);

                if (this->handshake_protocol->Failed())
                {
                    switch_to_state(State::handshake_protocol_failed);
                    return;
                }
                else
                {
                    switch_to_state(State::receive_record_state);
                }
            }
            break;

        case State::receive_record_state:
            switch_to_state(State::record_frame_pending_state);
            this->record_frame.Initialize(&this->record);
            break;

        case State::record_frame_pending_state:
            if (this->record_frame.Pending())
            {
                this->record_frame.Process(event, yield);
            }

            if (this->record_frame.Failed())
            {
                switch_to_state(State::record_frame_failed);
                return;
            }
            else if (this->record_frame.Succeeded())
            {
                try
                {
                    ProcessRecord(&this->record);
                }
                catch (State error_state)
                {
                    switch_to_state(error_state);
                    return;
                }

                switch_to_state(State::receive_record_state);
            }
            break;

        default:
            throw new Exception("Tls::RecordLayer::Process unexpected state");
        }
    }

    void RecordLayer::WriteAlert(AlertDescription description, AlertLevel level)
    {
        Alert alert;
        alert.description = description;
        alert.level = level;

        Inline<AlertFrame> alert_frame;
        alert_frame.Initialize(&alert);

        this->current_type = ContentType::alert;
        alert_frame.SerializeTo(this);

        Flush();
    }

    void RecordLayer::WriteEOF()
    {
        WriteAlert(AlertDescription::close_notify, AlertLevel::warning);

        CloseTransport();
    }

    void RecordLayer::CloseTransport()
    {
        this->transport_peer->WriteEOF();
        this->transport_peer = 0;
    }

    void RecordLayer::WriteChangeCipherSpec()
    {
        Write(ContentType::change_cipher_spec, &ChangeCipherSpec, sizeof(ChangeCipherSpec));

        // make sure to actually send this record prior to changing the write states
        FlushRecordBuffer();

        this->active_write_state = this->pending_write_state;
        this->pending_write_state = New<ConnectionState>();
    }

    void RecordLayer::ConnectApplication()
    {
        this->application_connected = true;

        this->current_type = ContentType::application_data;
        ReadyForWriteBytesEvent event;
        event.Initialize(&this->application_element_source);
        this->application_stream->Process(&event);

        if (this->application_stream->Failed())
            throw State::application_stream_failed;

        if (this->send_heartbeats)
        {
            HeartbeatMessage heartbeat_message;

            heartbeat_message.type = HeartbeatMessageType::heartbeat_response;

            // $$ heartbleed test could be here
            heartbeat_message.payload_length = 16;

            heartbeat_message.payload.resize(16);
            NTSTATUS error = BCryptGenRandom(0, &heartbeat_message.payload[0], heartbeat_message.payload.size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
            if (error != 0)
                throw new Exception("Tls::ClientHandshake::Process BCryptGenRandom failed", error);

            heartbeat_message.padding.resize(16);
            error = BCryptGenRandom(0, &heartbeat_message.padding[0], heartbeat_message.padding.size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
            if (error != 0)
                throw new Exception("Tls::ClientHandshake::Process BCryptGenRandom failed", error);

            Inline<HeartbeatMessageFrame> heartbeat_message_frame;
            heartbeat_message_frame.Initialize(&heartbeat_message, 0);

            this->current_type = ContentType::heartbeat_content_type;
            heartbeat_message_frame.SerializeTo(this);
            Flush();
        }
    }

    void RecordLayer::DisconnectApplication()
    {
        ElementStreamEndingEvent application_event;
        this->application_stream->Process(&application_event);
    }

    void RecordLayer::switch_to_state(uint32 state)
    {
        __super::switch_to_state(state);

        if (Failed())
        {
            DisconnectApplication();

            AlertDescription description;

            switch (state)
            {
            case State::decrypt_stream_mac_mismatch_error:
            case State::decrypt_block_padding_overflow_error:
            case State::decrypt_block_padding_invalid_error:
            case State::decrypt_block_mac_mismatch_error:
                break;

            default:
                description = AlertDescription::internal_error;
                break;
            }

            WriteAlert(description, AlertLevel::fatal);

            CloseTransport();
        }
    }

    void RecordLayer::ProcessRecord(Record* record)
    {
        // From http://www.ietf.org/rfc/rfc2246.txt section 6.2.1:
        //     The length (in bytes) of the following TLSPlaintext.fragment.
        //     The length should not exceed 2^14.
        // brian: since it is only _should_, and I see some records in real life come back at 0x4020, let's
        // give it double.
        if (record->length > 0x8000)
            throw State::record_length_too_large_error;

        if (this->version_finalized && record->version != this->version)
            throw State::record_version_mismatch_error;

        Record compressed;
        Decrypt(record, &compressed);

        Record plaintext;
        Decompress(&compressed, &plaintext);

        switch(plaintext.type)
        {
        case ContentType::change_cipher_spec:
            {
                // RFC 5246 section 6.2.1
                // Implementations MUST NOT send zero-length fragments of HandshakeProtocol,
                // Alert, or ChangeCipherSpec content types.  Zero-length fragments of
                // Application data MAY be sent as they are potentially useful as a
                // traffic analysis countermeasure.

                if (plaintext.length == 0)
                    throw State::change_cipher_spec_zero_fragment_error;

                this->active_read_state = this->pending_read_state;
                this->pending_read_state = New<ConnectionState>();

                this->current_type = ContentType::handshake;
                ChangeCipherSpecEvent event;
                this->handshake_protocol->Frame::Process(&event);

                if (this->handshake_protocol->Failed())
                    throw State::handshake_protocol_failed;
            }
            break;

        case ContentType::alert:
            {
                // RFC 5246 section 6.2.1
                // Implementations MUST NOT send zero-length fragments of HandshakeProtocol,
                // Alert, or ChangeCipherSpec content types.  Zero-length fragments of
                // Application data MAY be sent as they are potentially useful as a
                // traffic analysis countermeasure.

                if (plaintext.length == 0)
                    throw State::alert_zero_fragment_error;

                this->current_type = ContentType::alert;
                this->alert_element_source.Initialize(plaintext.fragment->FirstElement(), plaintext.length);
                ReadyForReadBytesEvent event;
                event.Initialize(&this->alert_element_source);
                this->alert_protocol->Frame::Process(&event);

                if (this->alert_protocol->Failed())
                    throw State::alert_protocol_failed;
            }
            break;

        case ContentType::heartbeat_content_type:
            {
                if (this->receive_heartbeats == false)
                    throw State::unexpected_heartbeat_error;

                if (plaintext.length == 0)
                    throw State::heartbeat_zero_fragment_error;

                this->current_type = ContentType::heartbeat_content_type;
                this->heartbeat_element_source.Initialize(plaintext.fragment->FirstElement(), plaintext.length);
                ReadyForReadBytesEvent event;
                event.Initialize(&this->heartbeat_element_source);
                this->heartbeat_protocol->SetPlaintextLength(plaintext.length);
                this->heartbeat_protocol->Frame::Process(&event);

                if (this->heartbeat_protocol->Failed())
                    throw State::heartbeat_protocol_failed;
            }
            break;

        case ContentType::handshake:
            {
                // RFC 5246 section 6.2.1
                // Implementations MUST NOT send zero-length fragments of HandshakeProtocol,
                // Alert, or ChangeCipherSpec content types.  Zero-length fragments of
                // Application data MAY be sent as they are potentially useful as a
                // traffic analysis countermeasure.

                if (plaintext.length == 0)
                    throw State::handshake_zero_fragment_error;

                this->current_type = ContentType::handshake;
                this->handshake_element_source.Initialize(plaintext.fragment->FirstElement(), plaintext.length);
                ReadyForReadBytesEvent event;
                event.Initialize(&this->handshake_element_source);
                this->handshake_protocol->Frame::Process(&event);

                if (this->handshake_protocol->Failed())
                    throw State::handshake_protocol_failed;
            }
            break;

        case ContentType::application_data:
            {
                if (!this->application_connected)
                    throw State::unexpected_application_data_error;

                // RFC 5246 section 6.2.1
                // Implementations MUST NOT send zero-length fragments of HandshakeProtocol,
                // Alert, or ChangeCipherSpec content types.  Zero-length fragments of
                // Application data MAY be sent as they are potentially useful as a
                // traffic analysis countermeasure.

                if (plaintext.length != 0)
                {
                    this->current_type = ContentType::application_data;
                    this->application_element_source.Initialize(plaintext.fragment->FirstElement(), plaintext.length);
                    ReadyForReadBytesEvent event;
                    event.Initialize(&this->application_element_source);
                    this->application_stream->Process(&event);

                    if (this->application_stream->Failed())
                        throw State::application_stream_failed;
                }
            }
            break;

        default:
            throw State::unexpected_record_type_error;
        }
    }

    void RecordLayer::Write(const byte* elements, uint32 count)
    {
        Write(this->current_type, elements, count);
    }

    void RecordLayer::Write(ContentType type, const byte* elements, uint32 count)
    {
        if (type != this->buffer_type)
        {
            if (this->record_buffer->size() > 0)
            {
                FlushRecordBuffer();
            }

            this->buffer_type = type;
        }

        // The record layer fragments information blocks into TLSPlaintext
        // records carrying data in chunks of 2^14 bytes or less.  Client
        // message boundaries are not preserved in the record layer (i.e.,
        // multiple client messages of the same ContentType MAY be coalesced
        // into a single TLSPlaintext record, or a single message MAY be
        // fragmented across several records).

        while (true)
        {
            uint32 remaining = 0x4000 - this->record_buffer->size();
            uint32 useable = (count > remaining) ? remaining : count;

            this->record_buffer->insert(this->record_buffer->end(), elements, elements + useable);

            elements += useable;
            count -= useable;

            if (this->record_buffer->size() == 0x4000)
            {
                FlushRecordBuffer();
            }

            if (count == 0)
                break;
        }
    }

    void RecordLayer::FlushRecordBuffer()
    {
        // RFC 5246 section 6.2.1
        // Implementations MUST NOT send zero-length fragments of HandshakeProtocol,
        // Alert, or ChangeCipherSpec content types.  Zero-length fragments of
        // Application data MAY be sent as they are potentially useful as a
        // traffic analysis countermeasure.

        if (this->buffer_type == ContentType::alert ||
            this->buffer_type == ContentType::change_cipher_spec ||
            this->buffer_type == ContentType::handshake)
        {
            if (this->record_buffer->size() == 0)
                throw State::send_zero_fragment_error;
        }

        Record plaintext;
        plaintext.type = this->buffer_type;
        plaintext.version = this->version;
        plaintext.fragment = this->record_buffer;
        plaintext.length = this->record_buffer->size();

        Record compressed;
        Compress(&plaintext, &compressed);

        Record encrypted;
        Encrypt(&compressed, &encrypted);

        Inline<RecordFrame> recordFrame;
        recordFrame.Initialize(&encrypted);

        recordFrame.SerializeTo(&this->transport_buffer);

        this->record_buffer->resize(0);
    }

    void RecordLayer::Flush()
    {
        if (this->record_buffer->size() > 0)
        {
            FlushRecordBuffer();
        }

        this->transport_buffer.SerializeTo(this->transport_peer);
        this->transport_buffer.clear();
        this->transport_peer->Flush();
    }

    void RecordLayer::Compress(Record* plaintext, Record* compressed)
    {
        compressed->type = plaintext->type;
        compressed->version = plaintext->version;

        switch(this->active_write_state->security_parameters->compression_algorithm)
        {
        case CompressionMethod::cm_null:
            compressed->fragment = plaintext->fragment;
            compressed->length = plaintext->length;
            break;

        default:
            throw State::compress_unexpected_compression_algorithm_error;
        }
    }

    void RecordLayer::Encrypt(Record* compressed, Record* encrypted)
    {
        switch(this->active_write_state->security_parameters->cipher_type)
        {
        case CipherType::stream:
            EncryptStream(compressed, encrypted);
            return;

        case CipherType::block:
            EncryptBlock(compressed, encrypted);
            return;

        default:
            throw State::encrypt_unexpected_cipher_type_error;
        }
    }

    void RecordLayer::EncryptStream(Record* compressed, Record* encrypted)
    {
        encrypted->type = compressed->type;
        encrypted->version = compressed->version;

        switch(this->active_write_state->security_parameters->bulk_cipher_algorithm)
        {
        case BulkCipherAlgorithm::bca_null:
            encrypted->fragment = compressed->fragment;
            encrypted->length = compressed->length;
            break;

        case BulkCipherAlgorithm::rc4:
            {
                uint8 mac_length = this->active_write_state->security_parameters->mac_length;

                std::vector<byte> payload;
                payload.reserve(compressed->fragment->size() + mac_length);

                payload.insert(payload.end(), compressed->fragment->begin(), compressed->fragment->end());

                int mac_index = payload.size();

                payload.resize(mac_index + mac_length);

                this->active_write_state->MAC(compressed, &payload[mac_index], mac_length);

                encrypted->fragment = New<ByteVector>();
                encrypted->fragment->resize(payload.size());

                uint32 output_length;

                NTSTATUS error = BCryptEncrypt(
                    this->active_write_state->key_handle,
                    &payload[0],
                    payload.size(),
                    0,
                    0,
                    0,
                    encrypted->fragment->FirstElement(),
                    encrypted->fragment->size(),
                    &output_length, 
                    0);
                if (error != 0)
                    throw new Exception("BCryptEncrypt", error);

                if (output_length > 0xffff)
                    throw State::encrypt_stream_output_overflow_error;

                encrypted->fragment->resize(output_length);
                encrypted->length = encrypted->fragment->size();

                this->active_write_state->sequence_number++;
            }
            break;

        default:
            throw State::encrypt_stream_unexpected_bulk_cipher_algorithm_error;
        }
    }

    void RecordLayer::EncryptBlock(Record* compressed, Record* encrypted)
    {
        encrypted->type = compressed->type;
        encrypted->version = compressed->version;

        switch(this->active_write_state->security_parameters->bulk_cipher_algorithm)
        {
        case BulkCipherAlgorithm::aes:
        case BulkCipherAlgorithm::_3des:
            {
                uint8 mac_length = this->active_write_state->security_parameters->mac_length;
                uint8 padding_length;

                std::vector<byte> payload;
                payload.reserve(compressed->fragment->size() + mac_length + sizeof(padding_length) + 0x100);

                ByteVector::Ref mask;
                uint8 record_iv_length;

                if (this->version <= 0x0301)
                {
                    record_iv_length = 0;

                    mask = this->active_write_state->IV;
                }
                else
                {
                    record_iv_length = this->active_write_state->security_parameters->block_length;

                    mask = New<ByteVector>();
                    mask->resize(record_iv_length);

                    NTSTATUS error = BCryptGenRandom(0, mask->FirstElement(), mask->size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
                    if (error != 0)
                        throw new Exception("Tls::ClientHandshake::Process BCryptGenRandom failed", error);
                }

                payload.insert(payload.end(), compressed->fragment->begin(), compressed->fragment->end());

                int mac_index = payload.size();

                payload.resize(mac_index + mac_length);

                this->active_write_state->MAC(compressed, &payload[mac_index], mac_length);

                uint16 unpadded_length = payload.size() + sizeof(padding_length);
                uint8 overflow = unpadded_length % this->active_write_state->security_parameters->block_length;
                padding_length = overflow == 0 ? 0 : this->active_write_state->security_parameters->block_length - overflow;

                payload.insert(payload.end(), padding_length + 1, padding_length);

                encrypted->fragment = New<ByteVector>();
                encrypted->fragment->reserve(record_iv_length + payload.size());

                if (record_iv_length > 0)
                {
                    encrypted->fragment->insert(encrypted->fragment->end(), mask->begin(), mask->end());
                }

                encrypted->fragment->resize(record_iv_length + payload.size());

                uint32 output_length;

                NTSTATUS error = BCryptEncrypt(
                    this->active_write_state->key_handle,
                    &payload[0],
                    payload.size(),
                    0,
                    mask->FirstElement(),
                    mask->size(),
                    encrypted->fragment->FirstElement() + record_iv_length,
                    encrypted->fragment->size() - record_iv_length,
                    &output_length, 
                    0);
                if (error != 0)
                    throw new Exception("BCryptEncrypt", error);

                if (output_length > 0xffff)
                    throw State::encrypt_block_output_overflow_error;

                encrypted->fragment->resize(record_iv_length + output_length);
                encrypted->length = encrypted->fragment->size();

                if (this->version <= 0x0301)
                {
                    ByteVector::Ref cbc_residue = New<ByteVector>();
                    cbc_residue->insert(cbc_residue->end(), encrypted->fragment->end() - this->active_write_state->security_parameters->block_length, encrypted->fragment->end());
                    this->active_write_state->IV = cbc_residue;
                }

                this->active_write_state->sequence_number++;
            }
            break;

        default:
            throw State::encrypt_block_unexpected_bulk_cipher_algorithm_error;
        }
    }

    void RecordLayer::Decompress(Record* compressed, Record* plaintext)
    {
        plaintext->type = compressed->type;
        plaintext->version = compressed->version;

        switch(this->active_read_state->security_parameters->compression_algorithm)
        {
        case CompressionMethod::cm_null:
            plaintext->fragment = compressed->fragment;
            plaintext->length = compressed->length;
            break;

        default:
            throw State::decompress_unexpected_compression_algorithm_error;
        }
    }

    void RecordLayer::Decrypt(Record* encrypted, Record* compressed)
    {
        switch(this->active_read_state->security_parameters->cipher_type)
        {
        case CipherType::stream:
            DecryptStream(encrypted, compressed);
            break;

        case CipherType::block:
            DecryptBlock(encrypted, compressed);
            break;

        default:
            throw State::decrypt_unexpected_cipher_type_error;
        }
    }

    void RecordLayer::DecryptStream(Record* encrypted, Record* compressed)
    {
        compressed->type = encrypted->type;
        compressed->version = encrypted->version;

        switch(this->active_read_state->security_parameters->bulk_cipher_algorithm)
        {
        case BulkCipherAlgorithm::bca_null:
            compressed->fragment = encrypted->fragment;
            compressed->length = encrypted->length;
            break;

        case BulkCipherAlgorithm::rc4:
            {
                ByteVector::Ref payload = New<ByteVector>();
                payload->resize(encrypted->fragment->size());

                uint32 output_length;
                NTSTATUS error = BCryptDecrypt(
                    this->active_read_state->key_handle, 
                    encrypted->fragment->FirstElement(), 
                    encrypted->fragment->size(), 
                    0, 
                    0, 
                    0, 
                    payload->FirstElement(), 
                    payload->size(), 
                    &output_length, 
                    0);
                if (error != 0)
                {
                    Basic::globals->HandleError("BCryptDecrypt", error);
                    throw State::decrypt_stream_decryption_failed;
                }

                payload->resize(output_length);

                uint8 mac_length = this->active_read_state->security_parameters->mac_length;

                uint16 fragment_length = payload->size() - mac_length;

                std::vector<opaque> received_MAC;
                received_MAC.insert(received_MAC.end(), payload->begin() + fragment_length, payload->begin() + fragment_length + mac_length);

                std::vector<opaque> expected_MAC;
                expected_MAC.resize(mac_length);

                payload->resize(fragment_length);
                
                compressed->fragment = payload;
                compressed->length = compressed->fragment->size();

                this->active_read_state->MAC(compressed, &expected_MAC[0], expected_MAC.size());

                for (int i = 0; i < mac_length; i++)
                {
                    if (received_MAC[i] != expected_MAC[i])
                        throw State::decrypt_stream_mac_mismatch_error;
                }

                this->active_read_state->sequence_number++;
            }
            break;

        default:
            throw State::decrypt_stream_unexpected_bulk_cipher_algorithm_error;
        }
    }

    void RecordLayer::DecryptBlock(Record* encrypted, Record* compressed)
    {
        compressed->type = encrypted->type;
        compressed->version = encrypted->version;

        switch(this->active_read_state->security_parameters->bulk_cipher_algorithm)
        {
        case BulkCipherAlgorithm::aes:
        case BulkCipherAlgorithm::_3des:
            {
                ByteVector::Ref mask;
                uint8 record_iv_length;

                if (this->version <= 0x0301)
                {
                    record_iv_length = 0;

                    mask = this->active_read_state->IV;
                }
                else
                {
                    record_iv_length = this->active_read_state->security_parameters->block_length;

                    mask = New<ByteVector>();
                    mask->insert(mask->end(), encrypted->fragment->begin(), encrypted->fragment->begin() + record_iv_length);
                }

                ByteVector::Ref payload = New<ByteVector>();
                payload->resize(encrypted->fragment->size() - record_iv_length);

                uint32 output_length;
                NTSTATUS error = BCryptDecrypt(
                    this->active_read_state->key_handle, 
                    encrypted->fragment->FirstElement() + record_iv_length, 
                    encrypted->fragment->size() - record_iv_length, 
                    0, 
                    mask->FirstElement(), 
                    mask->size(), 
                    payload->FirstElement(), 
                    payload->size(), 
                    &output_length, 
                    0);
                if (error != 0)
                {
                    Basic::globals->HandleError("BCryptDecrypt", error);
                    throw State::decrypt_block_decryption_failed;
                }

                payload->resize(output_length);

                uint8 padding_length = payload->at(payload->size() - 1);
                uint8 mac_length = this->active_read_state->security_parameters->mac_length;
                uint16 fixed_portion = mac_length + sizeof(padding_length);
                uint16 variable_portion = payload->size() - fixed_portion;

                // Implementation note: Canvel et al. [CBCTIME] have demonstrated a
                // timing attack on CBC padding based on the time required to compute
                // the MAC.  In order to defend against this attack, implementations
                // MUST ensure that record processing time is essentially the same
                // whether or not the padding is correct.  In general, the best way to
                // do this is to compute the MAC even if the padding is incorrect, and
                // only then reject the packet.  For instance, if the pad appears to be
                // incorrect, the implementation might assume a zero-length pad and then
                // compute the MAC.
                State canvel_error = State::done_state;

                if (padding_length > variable_portion)
                {
                    canvel_error = State::decrypt_block_padding_overflow_error;
                    padding_length = 0;
                }

                uint16 padding_index = payload->size() - padding_length - sizeof(padding_length);

                for (int i = 0; i < padding_length; i++)
                {
                    if (payload->at(padding_index + i) != padding_length)
                    {
                        if (canvel_error != State::done_state)
                            canvel_error = State::decrypt_block_padding_invalid_error;

                        padding_length = 0;
                    }
                }

                uint16 fragment_length = variable_portion - padding_length;

                std::vector<opaque> received_MAC;
                received_MAC.insert(received_MAC.end(), payload->begin() + fragment_length, payload->begin() + fragment_length + mac_length);

                std::vector<opaque> expected_MAC;
                expected_MAC.resize(mac_length);

                payload->resize(fragment_length);
                
                compressed->fragment = payload;
                compressed->length = compressed->fragment->size();

                this->active_read_state->MAC(compressed, &expected_MAC[0], expected_MAC.size());

                if (canvel_error != State::done_state)
                    throw canvel_error;

                for (int i = 0; i < mac_length; i++)
                {
                    if (received_MAC[i] != expected_MAC[i])
                        throw State::decrypt_block_mac_mismatch_error;
                }

                if (this->version <= 0x0301)
                {
                    ByteVector::Ref cbc_residue = New<ByteVector>();
                    cbc_residue->insert(cbc_residue->end(), encrypted->fragment->end() - this->active_read_state->security_parameters->block_length, encrypted->fragment->end());
                    this->active_read_state->IV = cbc_residue;
                }

                this->active_read_state->sequence_number++;
            }
            break;

        default:
            throw State::decrypt_block_unexpected_bulk_cipher_algorithm_error;
        }
    }

    void RecordLayer::FinalizeVersion(ProtocolVersion version)
    {
        if (this->version_finalized)
            throw State::version_already_finalized_error;

        this->version = version;
        this->version_finalized = true;

        // $ todo for tls 1.1:
        //-  Handling of padding errors is changed to use the bad_record_mac
        //   alert rather than the decrypt_stream_decryption_failed alert to protect against
        //   CBC attacks.

        //-  Premature closes no longer cause a session to be nonresumable.

        //-  Additional informational notes were added for various new attacks
        //   on TLS.

        // $ todo for tls 1.2:
        //-  The MD5/SHA-1 combination in the pseudorandom function (PRF) has
        //   been replaced with cipher-suite-specified PRFs.  All cipher suites
        //   in this document use P_SHA256.

        //-  The MD5/SHA-1 combination in the digitally-signed element has been
        //   replaced with a single hash.  Signed elements now include a field
        //   that explicitly specifies the hash algorithm used.

        //-  Substantial cleanup to the client's and server's ability to
        //   specify which hash and signature algorithms they will accept.
        //   Note that this also relaxes some of the constraints on signature
        //   and hash algorithms from previous versions of TLS.

        //-  Addition of support for authenticated encryption with additional
        //   data modes.

        //-  TLS Extensions definition and AES Cipher Suites were merged in
        //   from external [TLSEXT] and [TLSAES].

        //-  Tighter checking of EncryptedPreMasterSecret version numbers.

        //-  Tightened up a number of requirements.

        //-  Verify_data length now depends on the cipher suite (default is
        //   still 12).

        //-  Cleaned up description of Bleichenbacher/Klima attack defenses.

        //-  Alerts MUST now be sent in many cases.

        //-  After a certificate_request, if no certificates are available,
        //   clients now MUST send an empty certificate list.

        //-  TLS_RSA_WITH_AES_128_CBC_SHA is now the mandatory to implement
        //   cipher suite.

        //-  Added HMAC-SHA256 cipher suites.

        //-  Removed IDEA and DES cipher suites.  They are now deprecated and
        //   will be documented in a separate document.

        //-  Support for the SSLv2 backward-compatible hello is now a MAY, not
        //   a SHOULD, with sending it a SHOULD NOT.  Support will probably
        //   become a SHOULD NOT in the future.

        //-  Added limited "fall-through" to the presentation language to allow
        //   multiple case arms to have the same encoding.

        //-  Added an Implementation Pitfalls sections
    }
}