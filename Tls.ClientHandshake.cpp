// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.ClientHandshake.h"
#include "Basic.CountStream.h"
#include "Basic.HashAlgorithm.h"
#include "Basic.HashStream.h"
#include "Tls.ClientHelloFrame.h"
#include "Tls.RecordLayer.h"
#include "Tls.HandshakeFrame.h"
#include "Tls.PreMasterSecretFrame.h"
#include "Tls.RandomFrame.h"
#include "Tls.Globals.h"
#include "Tls.ServerNameFrame.h"
#include "Tls.SignatureAndHashAlgorithmFrame.h"

namespace Tls
{
    using namespace Basic;

    ClientHandshake::ClientHandshake(RecordLayer* session) :
        HandshakeProtocol(session),
        server_hello_frame(&this->serverHello), // initialization is in order of declaration in class def
        certificates_frame(&this->certificates) // initialization is in order of declaration in class def
    {
    }

    ProcessResult ClientHandshake::process_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::start_state:
            {
                if (event->get_type() != Basic::EventType::can_send_bytes_event)
                {
                    StateMachine::LogUnexpectedEvent("Tls", "ClientHandshake::process_event", event);
                    return ProcessResult::process_result_blocked;
                }

                Event::AddObserver<byte>(event, this->handshake_messages);

                this->security_parameters->client_random.gmt_unix_time = 0;

                NTSTATUS error = BCryptGenRandom(0, this->security_parameters->client_random.random_bytes, sizeof(this->security_parameters->client_random.random_bytes), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
                if (error != 0)
                    throw FatalError("Tls", "ClientHandshake::process_event BCryptGenRandom failed", error);

                ClientHello clientHello;
                clientHello.client_version = this->session->version_high;
                clientHello.random = this->security_parameters->client_random;
                clientHello.cipher_suites = Tls::globals->supported_cipher_suites;
                clientHello.compression_methods.push_back(CompressionMethod::cm_null);
                clientHello.heartbeat_extension_initialized = false; // $$

                Serializer<ClientHello> client_hello_serializer(&clientHello);

                bool success = WriteMessage(HandshakeType::client_hello, &client_hello_serializer);
                if (!success)
                {
                    switch_to_state(State::WriteMessage_1_failed);
                    return ProcessResult::process_result_ready;
                }

                // due to async nature, we can get called back even before write_record_elements returns, so be ready first...

                this->handshake_frame.reset();
                switch_to_state(State::expecting_server_hello_state);

                ByteStringRef send_buffer = std::make_shared<ByteString>();
                send_buffer.swap(this->send_buffer);

                this->session->write_record_elements(ContentType::handshake, send_buffer->address(), send_buffer->size());
                return ProcessResult::process_result_blocked; // event consumed
            }
            break;

        case State::expecting_server_hello_state:
            {
                ProcessResult result = process_event_change_state_on_fail(&this->handshake_frame, event, State::handshake_frame_1_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                if (this->handshake.msg_type != HandshakeType::server_hello)
                {
                    switch_to_state(State::expecting_server_hello_error);
                }
                else
                {
                    this->server_hello_frame.set_record_frame_length(this->handshake.length);
                    switch_to_state(State::server_hello_frame_pending_state);
                }
            }
            break;

        case State::server_hello_frame_pending_state:
            {
                ProcessResult result = process_event_change_state_on_fail(&this->server_hello_frame, event, State::server_hello_frame_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                if (this->serverHello.server_version < this->session->version_low || this->serverHello.server_version > this->session->version_high)
                {
                    switch_to_state(State::server_version_error);
                    return ProcessResult::process_result_ready;
                }

                this->session->FinalizeVersion(this->serverHello.server_version);

                this->security_parameters->server_random = this->serverHello.random;

                bool success = this->security_parameters->InitializeCipherSuite(this->session->version, this->serverHello.cipher_suite, &this->key_exchange_algorithm);
                if (!success)
                {
                    switch_to_state(State::InitializeCipherSuite_failed);
                    return ProcessResult::process_result_ready;
                }

                if (this->serverHello.heartbeat_extension_initialized)
                {
                    this->session->receive_heartbeats = true;

                    switch (this->serverHello.heartbeat_extension.mode)
                    {
                    case HeartbeatMode::peer_allowed_to_send:
                        this->session->send_heartbeats = true;
                        break;

                    case HeartbeatMode::peer_not_allowed_to_send:
                        this->session->send_heartbeats = false;
                        break;

                    default:
                        // RFC6520 section 2:
                        // Upon reception of an unknown mode, an error Alert message using
                        // illegal_parameter as its AlertDescription MUST be sent in response.

                        this->session->WriteAlert(AlertDescription::illegal_parameter, AlertLevel::fatal);
                        switch_to_state(State::unexpected_heartbeat_mode_error);
                        return ProcessResult::process_result_ready;
                    }
                }

                switch(this->key_exchange_algorithm)
                {
                case KeyExchangeAlgorithm::_KEA_RSA:
                    this->handshake_frame.reset();
                    switch_to_state(State::expecting_certificate_state);
                    break;

                // $$ implement DHE_DSS
                //case KeyExchangeAlgorithm::DHE_DSS:
                //    break;

                default:
                    switch_to_state(State::unexpected_key_exchange_algorithm_1_error);
                    return ProcessResult::process_result_ready;
                }
            }
            break;

        case State::expecting_certificate_state:
            {
                ProcessResult result = process_event_change_state_on_fail(&this->handshake_frame, event, State::handshake_frame_2_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                if (this->handshake.msg_type != HandshakeType::certificate)
                {
                    switch_to_state(State::expecting_certificate_error);
                }
                else
                {
                    switch_to_state(State::certificates_frame_pending_state);
                }
            }
            break;

        case State::certificates_frame_pending_state:
            {
                ProcessResult result = process_event_change_state_on_fail(&this->certificates_frame, event, State::certificates_frame_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                this->handshake_frame.reset();
                switch_to_state(State::expecting_server_hello_done_state);
            }
            break;

        case State::expecting_server_hello_done_state:
            {
                ProcessResult result = process_event_change_state_on_fail(&this->handshake_frame, event, State::handshake_frame_3_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                if (this->handshake.msg_type != HandshakeType::server_hello_done)
                {
                    switch_to_state(State::expecting_server_hello_done_error);
                }
                else if (this->handshake.length != 0)
                {
                    switch_to_state(State::handshake_length_1_error);
                }
                else
                {
                    Basic::PCCERT_CONTEXT cert = CertCreateCertificateContext(X509_ASN_ENCODING, this->certificates[0].address(), this->certificates[0].size());
                    if (cert == 0)
                    {
                        switch_to_state(State::CertCreateCertificateContext_failed);
                        return ProcessResult::process_result_ready;
                    }

                    Basic::BCRYPT_KEY_HANDLE public_key;
                    bool success = (bool)CryptImportPublicKeyInfoEx2(X509_ASN_ENCODING, &cert->pCertInfo->SubjectPublicKeyInfo, 0, 0, &public_key);
                    if (!success)
                    {
                        switch_to_state(State::CryptImportPublicKeyInfoEx2_failed);
                        return ProcessResult::process_result_ready;
                    }

                    PreMasterSecret pre_master_secret;
                    pre_master_secret.client_version = this->session->version_high;

                    NTSTATUS error = BCryptGenRandom(0, pre_master_secret.random, sizeof(pre_master_secret.random), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
                    if (error != 0)
                    {
                        Basic::LogDebug("Tls", "ClientHandshake::process_event BCryptGenRandom failed", error);
                        switch_to_state(State::BCryptGenRandom_failed);
                        return ProcessResult::process_result_ready;
                    }

                    serialize<PreMasterSecret>()(&pre_master_secret, &this->pre_master_secret_bytes);

                    // don't keep the pre_paster_secret in memory longer than necessary
                    ZeroMemory(&pre_master_secret, sizeof(pre_master_secret));

                    DWORD result_length = 0;

                    error = BCryptEncrypt(
                        public_key,
                        this->pre_master_secret_bytes.address(),
                        this->pre_master_secret_bytes.size(),
                        0,
                        0,
                        0,
                        0,
                        0,
                        &result_length,
                        BCRYPT_PAD_PKCS1);
                    if (error != 0)
                    {
                        Basic::LogDebug("Tls", "ClientHandshake::process_event BCryptEncrypt failed", error);
                        switch_to_state(State::BCryptEncrypt_1_failed);
                        return ProcessResult::process_result_ready;
                    }

                    EncryptedPreMasterSecret pre_master_secret_encrypted;
                    pre_master_secret_encrypted.resize(result_length);

                    error = BCryptEncrypt(
                        public_key,
                        this->pre_master_secret_bytes.address(),
                        this->pre_master_secret_bytes.size(),
                        0,
                        0,
                        0,
                        pre_master_secret_encrypted.address(),
                        pre_master_secret_encrypted.size(),
                        &result_length,
                        BCRYPT_PAD_PKCS1);
                    if (error != 0)
                    {
                        Basic::LogDebug("Tls", "ClientHandshake::process_event BCryptEncrypt failed", error);
                        switch_to_state(State::BCryptEncrypt_2_failed);
                        return ProcessResult::process_result_ready;
                    }

                    pre_master_secret_encrypted.resize(result_length);

                    CalculateKeys(&this->pre_master_secret_bytes);

                    this->pre_master_secret_bytes.clear();

                    Serializer<EncryptedPreMasterSecret> encrypted_pre_master_secret_serializer(&pre_master_secret_encrypted);

                    // $$ move this comment; do logging to a logging microservice instead of local file
                    // $$ move this comment; pass peer in write bytes ready path

                    success = WriteMessage(HandshakeType::client_key_exchange, &encrypted_pre_master_secret_serializer);
                    if (!success)
                    {
                        switch_to_state(State::WriteMessage_2_failed);
                        return ProcessResult::process_result_ready;
                    }

                    success = WriteFinished(&Tls::globals->client_finished_label);
                    if (!success)
                    {
                        switch_to_state(State::WriteMessage_3_failed);
                        return ProcessResult::process_result_ready;
                    }

                    this->finished_expected.resize(this->security_parameters->verify_data_length);

                    Event::RemoveObserver<byte>(event, this->handshake_messages);

                    CalculateVerifyData(&Tls::globals->server_finished_label, finished_expected.address(), (uint16)finished_expected.size());

                    ZeroMemory(this->handshake_messages->address(), this->handshake_messages->size());
                    this->handshake_messages->clear();

                    // due to async nature, we can get called back even before write_record_elements returns, so be ready first...

                    switch_to_state(State::expecting_cipher_change_state);

                    ByteStringRef send_buffer = std::make_shared<ByteString>();
                    send_buffer.swap(this->send_buffer);

                    this->session->write_record_elements(ContentType::handshake, send_buffer->address(), send_buffer->size());
                    return ProcessResult::process_result_blocked; // event consumed
                }
            }
            break;

        case State::expecting_cipher_change_state:
            {
                if (event->get_type() != Tls::EventType::change_cipher_spec_event)
                {
                    StateMachine::LogUnexpectedEvent("Tls", "ClientHandshake::process_event", event);
                    return ProcessResult::process_result_blocked;
                }

                this->handshake_frame.reset();
                switch_to_state(State::expecting_finished_state);
                return ProcessResult::process_result_blocked; // event consumed
            }
            break;

        case State::expecting_finished_state:
            {
                ProcessResult result = process_event_change_state_on_fail(&this->handshake_frame, event, State::handshake_frame_4_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                if (this->handshake.msg_type != HandshakeType::finished)
                {
                    switch_to_state(State::expecting_finished_error);
                }
                else if (this->handshake.length != this->security_parameters->verify_data_length)
                {
                    switch_to_state(State::handshake_length_2_error);
                }
                else
                {
                    this->finished_received.resize(this->security_parameters->verify_data_length);

                    this->finished_received_frame.reset(this->finished_received.address(), this->finished_received.size());
                    switch_to_state(State::finished_received_frame_pending_state);
                }
            }
            break;

        case State::finished_received_frame_pending_state:
            {
                ProcessResult result = process_event_change_state_on_fail(&this->finished_received_frame, event, State::finished_received_frame_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                for (uint32 i = 0; i < this->security_parameters->verify_data_length; i++)
                {
                    if (finished_received[i] != finished_expected[i])
                    {
                        switch_to_state(State::finished_received_error);
                        return ProcessResult::process_result_ready;
                    }
                }

                // $$$ log level?
                //Basic::globals->DebugWriter()->WriteFormat<0x100>("TLS client handshake successfully negotiated 0x%04X", this->session->version);
                //Basic::globals->DebugWriter()->WriteLine();

                // $ handle renegotiates, etc.
                switch_to_state(State::done_state);
            }
            break;

        default:
            throw FatalError("Tls", "ClientHandshake::process_event unhandled state");
        }

        return ProcessResult::process_result_ready;
    }

    void ClientHandshake::PartitionKeyMaterial(ByteString* key_material)
    {
        Tls::globals->Partition(key_material, this->security_parameters->mac_key_length, &this->session->pending_write_state->MAC_key);
        Tls::globals->Partition(key_material, this->security_parameters->mac_key_length, &this->session->pending_read_state->MAC_key);
        Tls::globals->Partition(key_material, this->security_parameters->enc_key_length, &this->session->pending_write_state->encryption_key);
        Tls::globals->Partition(key_material, this->security_parameters->enc_key_length, &this->session->pending_read_state->encryption_key);

        if (this->session->version <= 0x0301)
        {
            Tls::globals->Partition(key_material, this->security_parameters->block_length, this->session->pending_write_state->IV.get());
            Tls::globals->Partition(key_material, this->security_parameters->block_length, this->session->pending_read_state->IV.get());
        }
    }
}
