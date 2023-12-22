// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.RecordLayer.h"
#include "Tls.Globals.h"
#include "Tls.RandomFrame.h"
#include "Tls.ServerHandshake.h"
#include "Tls.ClientHandshake.h"
#include "Basic.Event.h"
#include "Tls.ServerNameFrame.h"
#include "Tls.SignatureAndHashAlgorithmFrame.h"

namespace Tls
{
    using namespace Basic;

    RecordLayer::RecordLayer(std::shared_ptr<IProcess> application_stream, bool server, std::shared_ptr<ICertificate> certificate) :
        record_frame(&record)
    {
        // $$ make member initializers
        this->application_stream = application_stream;
        this->application_connected = false;
        this->server = server;
        this->certificate = certificate;

        this->version_low = 0x0301;
        this->version_high = 0x0302;
        this->version = this->version_high;
        this->version_finalized = false;

        this->session_id.resize(32);

        this->send_heartbeats = false;
        this->receive_heartbeats = false;

        NTSTATUS error = BCryptGenRandom(0, this->session_id.address(), this->session_id.size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
        if (error != 0)
            throw FatalError("BCryptGenRandom", error);

        if (server)
        {
            std::shared_ptr<ServerHandshake> handshake_server = std::make_shared<ServerHandshake>(this);
            this->handshake_protocol = handshake_server;
        }
        else
        {
            std::shared_ptr<ClientHandshake> handshake_client = std::make_shared<ClientHandshake>(this);
            this->handshake_protocol = handshake_client;
        }

        this->alert_protocol = std::make_shared<AlertProtocol>(this);

        this->heartbeat_protocol = std::make_shared<HeartbeatProtocol>(this);

        std::shared_ptr<SecurityParameters> security_parameters = std::make_shared<SecurityParameters>();

        this->active_read_state = std::make_shared<ConnectionState>();
        this->active_read_state->security_parameters = security_parameters;

        this->active_write_state = std::make_shared<ConnectionState>();
        this->active_write_state->security_parameters = security_parameters;

        this->pending_read_state = std::make_shared<ConnectionState>();
        this->pending_write_state = std::make_shared<ConnectionState>();
    }

    void RecordLayer::set_transport(std::shared_ptr<IStream<byte> > peer)
    {
        this->transport = peer;
    }

    event_result RecordLayer::consider_event(IEvent* event)
    {
        if (event->get_type() == Basic::EventType::element_stream_ending_event)
        {
            DisconnectApplication();
            return event_result_yield; // event consumed
        }

        switch (get_state())
        {
        case State::unconnected_state:
            {
                if (event->get_type() != Basic::EventType::can_send_bytes_event)
                {
                    HandleError("unexpected event");
                    return event_result_yield; // unexpected event
                }

                // produce same event but with specific element source so that handshake_protocol can AddObserver
                CanSendBytesEvent handshake_event;
                handshake_event.Initialize(&this->handshake_element_source);
                produce_event(this->handshake_protocol.get(), &handshake_event);

                if (this->handshake_protocol->failed())
                {
                    switch_to_state(State::handshake_protocol_failed);
                }
                else
                {
                    switch_to_state(State::receive_record_state);
                }

                return event_result_yield; // event consumed
            }
            break;

        case State::receive_record_state:
            this->record_frame.reset();
            switch_to_state(State::record_frame_pending_state);
            break;

        case State::record_frame_pending_state:
            {
                event_result result = delegate_event_change_state_on_fail(&this->record_frame, event, State::record_frame_failed);
                if (result == event_result_yield)
                    return event_result_yield;

                try // $$ remove dependency on exceptions
                {
                    ProcessRecord(&this->record);
                }
                catch (State error_state)
                {
                    switch_to_state(error_state);
                    return event_result_continue;
                }

                switch_to_state(State::receive_record_state);
            }
            break;

        default:
            throw FatalError("Tls::RecordLayer::handle_event unexpected state");
        }

        return event_result_continue;
    }

    void RecordLayer::WriteAlert(AlertDescription description, AlertLevel level)
    {
        Alert alert;
        alert.description = description;
        alert.level = level;

        ByteString alert_bytes;
        serialize<Alert>()(&alert, &alert_bytes);

        write_record_elements(ContentType::alert, alert_bytes.address(), alert_bytes.size());
    }

    void RecordLayer::write_eof()
    {
        WriteAlert(AlertDescription::close_notify, AlertLevel::warning);

        CloseTransport();
    }

    void RecordLayer::CloseTransport()
    {
        this->transport->write_eof();
        this->transport.reset();
    }

    void RecordLayer::WriteChangeCipherSpec()
    {
        write_record_elements(ContentType::change_cipher_spec, &ChangeCipherSpec, sizeof(ChangeCipherSpec));

        this->active_write_state = this->pending_write_state;
        this->pending_write_state = std::make_shared<ConnectionState>();
    }

    void RecordLayer::ConnectApplication()
    {
        std::shared_ptr<IProcess> protocol = this->application_stream.lock();
        if (protocol.get() == 0)
            throw State::application_lost_error_1;

        this->application_connected = true;
        CanSendBytesEvent event;
        event.Initialize(&this->application_element_source);
        produce_event(protocol.get(), &event);

        if (protocol->failed())
            throw State::application_stream_failed;

        if (this->send_heartbeats)
        {
            HeartbeatMessage heartbeat_message;

            heartbeat_message.type = HeartbeatMessageType::heartbeat_response;

            // $$ heartbleed test could be here
            heartbeat_message.payload_length = 16;

            heartbeat_message.payload.resize(16);
            NTSTATUS error = BCryptGenRandom(0, heartbeat_message.payload.address(), heartbeat_message.payload.size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
            if (error != 0)
                throw FatalError("Tls::ClientHandshake::handle_event BCryptGenRandom failed", error);

            heartbeat_message.padding.resize(16);
            error = BCryptGenRandom(0, heartbeat_message.padding.address(), heartbeat_message.padding.size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
            if (error != 0)
                throw FatalError("Tls::ClientHandshake::handle_event BCryptGenRandom failed", error);

            ByteString heartbeat_bytes;
            serialize<HeartbeatMessage>()(&heartbeat_message, &heartbeat_bytes);
            this->write_record_elements(ContentType::heartbeat_content_type, heartbeat_bytes.address(), heartbeat_bytes.size());
        }
    }

    void RecordLayer::DisconnectApplication()
    {
        std::shared_ptr<IProcess> protocol = this->application_stream.lock();
        if (protocol.get() == 0)
             return;

        ElementStreamEndingEvent application_event;
        produce_event(protocol.get(), &application_event);
    }

    void RecordLayer::switch_to_state(uint32 state)
    {
        __super::switch_to_state(state);

        if (failed())
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
        //     The length should not exceed 2^14 (0x4000).
        // brian: since it is only _should_, and I see some records in real life come back at 0x4020, let's
        // give it double.
        if (record->length > 0x8000)
            throw State::receive_record_length_too_large_error;

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
                this->pending_read_state = std::make_shared<ConnectionState>();

                ChangeCipherSpecEvent event;
                produce_event(this->handshake_protocol.get(), &event);

                if (this->handshake_protocol->failed())
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

                this->alert_element_source.Initialize(plaintext.fragment->address(), plaintext.length);

                ReceivedBytesEvent event;
                event.Initialize(&this->alert_element_source);
                produce_event(this->alert_protocol.get(), &event);

                if (this->alert_protocol->failed())
                    throw State::alert_protocol_failed;
            }
            break;

        case ContentType::heartbeat_content_type:
            {
                if (this->receive_heartbeats == false)
                    throw State::unexpected_heartbeat_error;

                if (plaintext.length == 0)
                    throw State::heartbeat_zero_fragment_error;

                this->heartbeat_protocol->SetPlaintextLength(plaintext.length);

                this->heartbeat_element_source.Initialize(plaintext.fragment->address(), plaintext.length);

                ReceivedBytesEvent event;
                event.Initialize(&this->heartbeat_element_source);
                produce_event(this->heartbeat_protocol.get(), &event);

                if (this->heartbeat_protocol->failed())
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

                this->handshake_element_source.Initialize(plaintext.fragment->address(), plaintext.length);

                ReceivedBytesEvent event;
                event.Initialize(&this->handshake_element_source);
                produce_event(this->handshake_protocol.get(), &event);

                if (this->handshake_protocol->failed())
                    throw State::handshake_protocol_failed;
            }
            break;

        case ContentType::application_data:
            {
                if (!this->application_connected)
                    throw State::unexpected_application_data_error;

                std::shared_ptr<IProcess> protocol = this->application_stream.lock();
                if (protocol.get() == 0)
                    throw State::application_lost_error_2;

                // RFC 5246 section 6.2.1
                // Implementations MUST NOT send zero-length fragments of HandshakeProtocol,
                // Alert, or ChangeCipherSpec content types.  Zero-length fragments of
                // Application data MAY be sent as they are potentially useful as a
                // traffic analysis countermeasure.

                if (plaintext.length != 0)
                {
                    this->application_element_source.Initialize(plaintext.fragment->address(), plaintext.length);

                    ReceivedBytesEvent event;
                    event.Initialize(&this->application_element_source);
                    produce_event(protocol.get(), &event);

                    if (protocol->failed())
                        throw State::application_stream_failed;
                }
            }
            break;

        default:
            throw State::unexpected_record_type_error;
        }
    }

    void RecordLayer::write_elements(const byte* elements, uint32 count)
    {
        write_record_elements(ContentType::application_data, elements, count);
    }

    void RecordLayer::write_record_elements(ContentType type, const byte* elements, uint32 count)
    {
        // The record layer fragments information blocks into TLSPlaintext
        // records carrying data in chunks of 2^14 bytes or less.  Client
        // message boundaries are not preserved in the record layer (i.e.,
        // multiple client messages of the same ContentType MAY be coalesced
        // into a single TLSPlaintext record, or a single message MAY be
        // fragmented across several records).

        while (true)
        {
            uint32 useable = (count > max_record_length) ? max_record_length : count;

            send_record(type, elements, useable);

            elements += useable;
            count -= useable;

            if (count == 0)
                break;
        }
    }

    void RecordLayer::send_record(ContentType type, const byte* elements, uint32 count)
    {
        // RFC 5246 section 6.2.1
        // Implementations MUST NOT send zero-length fragments of HandshakeProtocol,
        // Alert, or ChangeCipherSpec content types.  Zero-length fragments of
        // Application data MAY be sent as they are potentially useful as a
        // traffic analysis countermeasure.

        if (type == ContentType::alert ||
            type == ContentType::change_cipher_spec ||
            type == ContentType::handshake)
        {
            if (count == 0)
                throw State::send_zero_fragment_error;
        }

        if (count > max_record_length)
            throw State::send_record_length_too_large_error;

        // $$ optimize amount of buffering/copying in this section - probably have send_record take a ByteString directly,
        // make sure it has enough reserved space, and use directly all the way through to transport.  no copying or further
        // allocations required?

        ByteStringRef buffer = std::make_shared<ByteString>();
        buffer->write_elements(elements, count);

        Record plaintext;
        plaintext.type = type;
        plaintext.version = this->version;
        plaintext.fragment = buffer;
        plaintext.length = (uint16)buffer->size();

        Record compressed;
        Compress(&plaintext, &compressed);

        Record encrypted;
        Encrypt(&compressed, &encrypted);

        ByteString transport_buffer;
        serialize<Record>()(&encrypted, &transport_buffer);

        transport_buffer.write_to_stream(this->transport.get());
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

                ByteString payload;
                payload.reserve(compressed->fragment->size() + mac_length);

                payload.insert(payload.end(), compressed->fragment->begin(), compressed->fragment->end());

                int mac_index = payload.size();

                payload.resize(mac_index + mac_length);

                this->active_write_state->MAC(compressed, payload.address() + mac_index, mac_length);

                encrypted->fragment = std::make_shared<ByteString>();
                encrypted->fragment->resize(payload.size());

                uint32 output_length;

                NTSTATUS error = BCryptEncrypt(
                    this->active_write_state->key_handle,
                    payload.address(),
                    payload.size(),
                    0,
                    0,
                    0,
                    encrypted->fragment->address(),
                    encrypted->fragment->size(),
                    &output_length, 
                    0);
                if (error != 0)
                    throw FatalError("BCryptEncrypt", error);

                if (output_length > 0xffff)
                    throw State::encrypt_stream_output_overflow_error;

                encrypted->fragment->resize(output_length);
                encrypted->length = (uint16)encrypted->fragment->size();

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

                ByteString payload;
                payload.reserve(compressed->fragment->size() + mac_length + sizeof(padding_length) + 0x100);

                std::shared_ptr<ByteString> mask;
                uint8 record_iv_length;

                if (this->version <= 0x0301)
                {
                    record_iv_length = 0;

                    mask = this->active_write_state->IV;
                }
                else
                {
                    record_iv_length = this->active_write_state->security_parameters->block_length;

                    mask = std::make_shared<ByteString>();
                    mask->resize(record_iv_length);

                    NTSTATUS error = BCryptGenRandom(0, mask->address(), mask->size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
                    if (error != 0)
                        throw FatalError("Tls::ClientHandshake::handle_event BCryptGenRandom failed", error);
                }

                payload.insert(payload.end(), compressed->fragment->begin(), compressed->fragment->end());

                int mac_index = payload.size();

                payload.resize(mac_index + mac_length);

                this->active_write_state->MAC(compressed, &payload[mac_index], mac_length);

                uint16 unpadded_length = (uint16)payload.size() + sizeof(padding_length);
                uint8 overflow = unpadded_length % this->active_write_state->security_parameters->block_length;
                padding_length = overflow == 0 ? 0 : this->active_write_state->security_parameters->block_length - overflow;

                payload.insert(payload.end(), padding_length + 1, padding_length);

                encrypted->fragment = std::make_shared<ByteString>();
                encrypted->fragment->reserve(record_iv_length + payload.size());

                if (record_iv_length > 0)
                {
                    encrypted->fragment->insert(encrypted->fragment->end(), mask->begin(), mask->end());
                }

                encrypted->fragment->resize(record_iv_length + payload.size());

                uint32 output_length;

                NTSTATUS error = BCryptEncrypt(
                    this->active_write_state->key_handle,
                    payload.address(),
                    payload.size(),
                    0,
                    mask->address(),
                    mask->size(),
                    encrypted->fragment->address() + record_iv_length,
                    encrypted->fragment->size() - record_iv_length,
                    &output_length, 
                    0);
                if (error != 0)
                    throw FatalError("BCryptEncrypt", error);

                if (output_length > 0xffff)
                    throw State::encrypt_block_output_overflow_error;

                encrypted->fragment->resize(record_iv_length + output_length);
                encrypted->length = (uint16)encrypted->fragment->size();

                if (this->version <= 0x0301)
                {
                    std::shared_ptr<ByteString> cbc_residue = std::make_shared<ByteString>();
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
                std::shared_ptr<ByteString> payload = std::make_shared<ByteString>();
                payload->resize(encrypted->fragment->size());

                uint32 output_length;
                NTSTATUS error = BCryptDecrypt(
                    this->active_read_state->key_handle, 
                    encrypted->fragment->address(), 
                    encrypted->fragment->size(), 
                    0, 
                    0, 
                    0, 
                    payload->address(), 
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

                uint16 fragment_length = (uint16)payload->size() - mac_length;

                ByteString received_MAC;
                received_MAC.insert(received_MAC.end(), payload->begin() + fragment_length, payload->begin() + fragment_length + mac_length);

                ByteString expected_MAC;
                expected_MAC.resize(mac_length);

                payload->resize(fragment_length);
                
                compressed->fragment = payload;
                compressed->length = (uint16)compressed->fragment->size();

                this->active_read_state->MAC(compressed, expected_MAC.address(), (uint8)expected_MAC.size());

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
                std::shared_ptr<ByteString> mask;
                uint8 record_iv_length;

                if (this->version <= 0x0301)
                {
                    record_iv_length = 0;

                    mask = this->active_read_state->IV;
                }
                else
                {
                    record_iv_length = this->active_read_state->security_parameters->block_length;

                    mask = std::make_shared<ByteString>();
                    mask->insert(mask->end(), encrypted->fragment->begin(), encrypted->fragment->begin() + record_iv_length);
                }

                std::shared_ptr<ByteString> payload = std::make_shared<ByteString>();
                payload->resize(encrypted->fragment->size() - record_iv_length);

                uint32 output_length;
                NTSTATUS error = BCryptDecrypt(
                    this->active_read_state->key_handle, 
                    encrypted->fragment->address() + record_iv_length,
                    encrypted->fragment->size() - record_iv_length, 
                    0, 
                    mask->address(), 
                    mask->size(), 
                    payload->address(),
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
                uint16 variable_portion = (uint16)payload->size() - fixed_portion;

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

                uint16 padding_index = (uint16)payload->size() - padding_length - sizeof(padding_length);

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

                ByteString received_MAC;
                received_MAC.insert(received_MAC.end(), payload->begin() + fragment_length, payload->begin() + fragment_length + mac_length);

                ByteString expected_MAC;
                expected_MAC.resize(mac_length);

                payload->resize(fragment_length);
                
                compressed->fragment = payload;
                compressed->length = (uint16)compressed->fragment->size();

                this->active_read_state->MAC(compressed, expected_MAC.address(), (uint8)expected_MAC.size());

                if (canvel_error != State::done_state)
                    throw canvel_error;

                for (int i = 0; i < mac_length; i++)
                {
                    if (received_MAC[i] != expected_MAC[i])
                        throw State::decrypt_block_mac_mismatch_error;
                }

                if (this->version <= 0x0301)
                {
                    std::shared_ptr<ByteString> cbc_residue = std::make_shared<ByteString>();
                    cbc_residue->append(encrypted->fragment->end() - this->active_read_state->security_parameters->block_length, encrypted->fragment->end());
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