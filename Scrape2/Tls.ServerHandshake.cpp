// Copyright � 2013 Brian Spanton

#include "stdafx.h"
#include "Tls.ServerHandshake.h"
#include "Basic.CountStream.h"
#include "Basic.HashAlgorithm.h"
#include "Basic.HashStream.h"
#include "Basic.Cng.h"
#include "Tls.RecordLayer.h"
#include "Tls.HandshakeFrame.h"
#include "Tls.PreMasterSecretFrame.h"
#include "Tls.RandomFrame.h"
#include "Tls.Globals.h"
#include "Tls.ServerHelloFrame.h"
#include "Tls.ServerNameFrame.h"
#include "Tls.SignatureAndHashAlgorithmFrame.h"
#include "Basic.TextWriter.h"

namespace Tls
{
    using namespace Basic;

    ServerHandshake::ServerHandshake(RecordLayer* session) :
        HandshakeProtocol(session),
        client_hello_frame(&this->clientHello), // initialization is in order of declaration in class def
        pre_master_secret_frame(&this->pre_master_secret_bytes) // initialization is in order of declaration in class def
    {
    }

	void ServerHandshake::transport_connected()
	{
		switch (this->get_state())
		{
		case State::start_state:
			this->switch_to_state(State::expecting_client_hello_state);
			return;

		default:
			switch_to_state(unexpected_state_1_error);
			return;
		}
	}

	void ServerHandshake::transport_disconnected()
	{
	}

	void ServerHandshake::change_cipher_spec_event()
	{
		switch (this->get_state())
		{
		case State::expecting_cipher_change_state:
			this->handshake_frame.reset();
			switch_to_state(State::expecting_finished_state);
			return;

		default:
			switch_to_state(unexpected_state_2_error);
			return;
		}
	}

	ConsumeElementsResult ServerHandshake::consume_elements(IElementSource<byte>* element_source)
    {
		ObservableElementSource<byte> observed_element_source(element_source);

		observed_element_source.AddObserver(this->handshake_messages);
		element_source = &observed_element_source;

		switch (this->get_state())
        {
        case State::expecting_client_hello_state:
        {
			ConsumeElementsResult result = Basic::consume_elements(&this->handshake_frame, element_source, this, State::handshake_frame_1_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

            if (this->handshake.msg_type != HandshakeType::client_hello)
            {
                switch_to_state(State::expecting_client_hello_error);
				return ConsumeElementsResult::failed;
            }

            this->client_hello_frame.set_record_frame_length(this->handshake.length);

            switch_to_state(State::hello_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::hello_frame_pending_state:
        {
            ConsumeElementsResult result = Basic::consume_elements(&this->client_hello_frame, element_source, this, State::hello_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

            if (this->clientHello.client_version < this->session->version_low)
            {
                switch_to_state(State::client_version_error);
				return ConsumeElementsResult::failed;
			}

            if (this->clientHello.client_version > this->session->version_high)
            {
                this->session->FinalizeVersion(this->session->version_high);
            }
            else
            {
                this->session->FinalizeVersion(this->clientHello.client_version);
            }

            CipherSuite cipher_suite;
            bool success = Tls::globals->SelectCipherSuite(&this->clientHello.cipher_suites, &cipher_suite);
            if (!success)
            {
                switch_to_state(State::SelectCipherSuite_failed);
				return ConsumeElementsResult::failed;
			}

            success = this->security_parameters->InitializeCipherSuite(this->session->version, cipher_suite, &this->key_exchange_algorithm);
            if (!success)
            {
                switch_to_state(State::InitializeCipherSuite_failed);
				return ConsumeElementsResult::failed;
			}

            SignatureAndHashAlgorithms supported_signature_algorithms = this->clientHello.supported_signature_algorithms;
            if (supported_signature_algorithms.size() == 0)
            {
                // -  If the negotiated key exchange algorithm is one of (RSA, DHE_RSA,
                //     DH_RSA, RSA_PSK, ECDH_RSA, ECDHE_RSA), behave as if client had
                //     sent the value {sha1,rsa}.
                if (this->key_exchange_algorithm == KeyExchangeAlgorithm::_KEA_RSA ||
                    this->key_exchange_algorithm == KeyExchangeAlgorithm::DHE_RSA ||
                    this->key_exchange_algorithm == KeyExchangeAlgorithm::DH_RSA ||
                    this->key_exchange_algorithm == KeyExchangeAlgorithm::RSA_PSK ||
                    this->key_exchange_algorithm == KeyExchangeAlgorithm::ECDH_RSA ||
                    this->key_exchange_algorithm == KeyExchangeAlgorithm::ECDHE_RSA)
                {
                    SignatureAndHashAlgorithm alg = {sha1, _sa_rsa};
                    supported_signature_algorithms.push_back(alg);
                }

                // -  If the negotiated key exchange algorithm is one of (DHE_DSS,
                //     DH_DSS), behave as if the client had sent the value {sha1,dsa}.
                else if (this->key_exchange_algorithm == KeyExchangeAlgorithm::DHE_DSS ||
                    this->key_exchange_algorithm == KeyExchangeAlgorithm::DH_DSS)
                {
                    SignatureAndHashAlgorithm alg = {sha1, dsa};
                    supported_signature_algorithms.push_back(alg);
                }

                // -  If the negotiated key exchange algorithm is one of (ECDH_ECDSA,
                //     ECDHE_ECDSA), behave as if the client had sent value {sha1,ecdsa}.
                else if (this->key_exchange_algorithm == KeyExchangeAlgorithm::ECDH_ECDSA ||
                    this->key_exchange_algorithm == KeyExchangeAlgorithm::ECDHE_ECDSA)
                {
                    SignatureAndHashAlgorithm alg = {sha1, ecdsa};
                    supported_signature_algorithms.push_back(alg);
                }
            }

            this->security_parameters->client_random = this->clientHello.random;
            this->security_parameters->server_random.gmt_unix_time = 0;

            NTSTATUS error = BCryptGenRandom(0, this->security_parameters->server_random.random_bytes, sizeof(this->security_parameters->server_random.random_bytes), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
            if (error != 0)
            {
                Basic::globals->HandleError("ServerHandshake::handle_event BCryptGenRandom", error);
                switch_to_state(State::BCryptGenRandom_1_failed);
				return ConsumeElementsResult::failed;
			}

            ServerHello serverHello;
            serverHello.cipher_suite = cipher_suite;
            serverHello.compression_method = CompressionMethod::cm_null;
            serverHello.server_version = this->session->version;
            serverHello.session_id = this->session->session_id;
            serverHello.random = this->security_parameters->server_random;
            serverHello.heartbeat_extension_initialized = false; // $$

            if (this->clientHello.heartbeat_extension_initialized)
            {
                this->session->receive_heartbeats = true;

                switch (this->clientHello.heartbeat_extension.mode)
                {
                case HeartbeatMode::peer_allowed_to_send:
                    this->session->send_heartbeats = true;
                    break;

                case HeartbeatMode::peer_not_allowed_to_send:
                    this->session->send_heartbeats = false;
                    break;

                default:
                    // RFC 6520 section 2:
                    // Upon reception of an unknown mode, an error Alert message using
                    // illegal_parameter as its AlertDescription MUST be sent in response.

                    this->session->WriteAlert(AlertDescription::illegal_parameter, AlertLevel::fatal);
                    switch_to_state(State::unexpected_heartbeat_mode_error);
					return ConsumeElementsResult::failed;
				}
            }

            Serializer<ServerHello> server_hello_serializer(&serverHello);

            WriteMessage(HandshakeType::server_hello, &server_hello_serializer);

            //bool send_cert; // $$ nyi
            //bool send_dh; // $$ nyi

            switch (this->key_exchange_algorithm)
            {
            case KeyExchangeAlgorithm::_KEA_RSA:
                {
                    Serializer<Certificates> certificates_serializer(session->certificate->Certificates());

                    WriteMessage(HandshakeType::certificate, &certificates_serializer);
                }
                break;

            // $$ implement DHE_DSS
            //case KeyExchangeAlgorithm::DHE_DSS:
            //    {
            //        std::shared_ptr<CertificatesFrame> frame = std::make_shared<Tls::CertificatesFrame>();
            //        frame->Initialize(session->certificate->Certificates());

            //        WriteMessage(HandshakeType::certificate, frame);

            //        std::shared_ptr<ServerKeyExchangeFrame> frame = std::make_shared<Tls::ServerKeyExchangeFrame>();
            //        frame->Initialize();

            //        WriteMessage(HandshakeType::server_key_exchange, frame);
            //    }
            //    break;

            default:
                switch_to_state(State::unexpected_key_exchange_algorithm_2_error);
				return ConsumeElementsResult::failed;
			}

            WriteMessage(HandshakeType::server_hello_done, 0);

            this->handshake_frame.reset();
            switch_to_state(State::expecting_client_key_exchange_state);

            ByteStringRef send_buffer = std::make_shared<ByteString>();
            send_buffer.swap(this->send_buffer);

            this->session->write_record_elements(ContentType::handshake, send_buffer->address(), send_buffer->size());

			return ConsumeElementsResult::in_progress;
		}

        case State::expecting_client_key_exchange_state:
        {
            ConsumeElementsResult result = Basic::consume_elements(&this->handshake_frame, element_source, this, State::handshake_frame_2_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

            if (this->handshake.msg_type != HandshakeType::client_key_exchange)
            {
                switch_to_state(State::expecting_client_key_exchange_error);
				return ConsumeElementsResult::failed;
			}

            switch (this->key_exchange_algorithm)
            {
            case KeyExchangeAlgorithm::_KEA_RSA:
                switch_to_state(State::pre_master_secret_frame_pending);
				return ConsumeElementsResult::in_progress;

            // $$ implement DHE_DSS
            //case KeyExchangeAlgorithm::DHE_DSS:
            //    {
            //        this->pre_master_secret_bytes = std::make_shared<ByteString>();
            //        this->pre_master_secret_frame.Initialize(this->pre_master_secret_bytes);

            //        switch_to_state(State::client_diffie_hellman_public_value_frame_pending);
            //    }
            //    break;

            default:
                switch_to_state(State::unexpected_key_exchange_algorithm_1_error);
				return ConsumeElementsResult::failed;
			}
        }

        case State::pre_master_secret_frame_pending:
        {
            ConsumeElementsResult result = Basic::consume_elements(&this->pre_master_secret_frame, element_source, this, State::pre_master_secret_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

            bool success = ProcessClientKeyExchange(this->key_exchange_algorithm);
            if (!success)
            {
                // An attack discovered by Daniel Bleichenbacher [BLEI] can be used
                // to attack a TLS server which is using PKCS#1 encoded RSA. The
                // attack takes advantage of the fact that by failing in different
                // ways, a TLS server can be coerced into revealing whether a
                // particular message, when decrypted, is properly PKCS#1 formatted
                // or not.
                // 
                // The best way to avoid vulnerability to this attack is to treat
                // incorrectly formatted messages in a manner indistinguishable from
                // correctly formatted RSA blocks. Thus, when it receives an
                // incorrectly formatted RSA block, a server should generate a
                // random 48-byte value and proceed using it as the premaster
                // secret. Thus, the server will act identically whether the
                // received RSA block is correctly encoded or not.

                this->pre_master_secret_bytes.resize(48);

                NTSTATUS error = BCryptGenRandom(0, this->pre_master_secret_bytes.address(), this->pre_master_secret_bytes.size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
                if (error != 0)
                {
                    Basic::globals->HandleError("ServerHandshake::handle_event BCryptGenRandom", error);
                    switch_to_state(State::BCryptGenRandom_2_failed);
					return ConsumeElementsResult::failed;
				}
            }

            CalculateKeys(&this->pre_master_secret_bytes);

            this->pre_master_secret_bytes.clear();

            this->finished_expected.resize(this->security_parameters->verify_data_length);

            CalculateVerifyData(&Tls::globals->client_finished_label, finished_expected.address(), (uint16)finished_expected.size());

            switch_to_state(State::expecting_cipher_change_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::expecting_finished_state:
        {
            ConsumeElementsResult result = Basic::consume_elements(&this->handshake_frame, element_source, this, State::handshake_frame_3_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

            if (this->handshake.msg_type != HandshakeType::finished)
            {
                switch_to_state(State::expecting_finished_error);
				return ConsumeElementsResult::failed;
			}

            if (this->handshake.length != this->security_parameters->verify_data_length)
            {
                switch_to_state(State::handshake_length_error);
				return ConsumeElementsResult::failed;
			}

            this->finished_received.resize(this->security_parameters->verify_data_length);
            this->finished_received_frame.reset(this->finished_received.address(), this->finished_received.size());

            switch_to_state(State::finished_received_frame_pending_state);
			return ConsumeElementsResult::in_progress;
		}

        case State::finished_received_frame_pending_state:
        {
            ConsumeElementsResult result = Basic::consume_elements(&this->finished_received_frame, element_source, this, State::finished_received_frame_failed);
			if (result != ConsumeElementsResult::succeeded)
				return result;

            for (uint32 i = 0; i < this->security_parameters->verify_data_length; i++)
            {
                if (finished_received[i] != finished_expected[i])
                {
                    switch_to_state(State::finished_received_error);
					return ConsumeElementsResult::failed;
				}
            }

            WriteFinished(&Tls::globals->server_finished_label);

            TextWriter(Basic::globals->LogStream()).write_format<0x100>("TLS server handshake successfully negotiated 0x%04X", this->session->version);
            TextWriter(Basic::globals->LogStream()).write_line();

            // $ handle renegotiates, etc.
            switch_to_state(State::done_state);

            ByteStringRef send_buffer = std::make_shared<ByteString>();
            send_buffer.swap(this->send_buffer);

            this->session->write_record_elements(ContentType::handshake, send_buffer->address(), send_buffer->size());

			return ConsumeElementsResult::in_progress;
		}

        default:
			switch_to_state(unexpected_state_3_error);
			return ConsumeElementsResult::failed;
		}
    }

    bool ServerHandshake::ProcessClientKeyExchange(KeyExchangeAlgorithm key_exchange_algorithm)
    {
        switch (key_exchange_algorithm)
        {
        case KeyExchangeAlgorithm::_KEA_RSA:
            {
                DWORD result_length = 0;

                bool success = this->session->certificate->CertDecrypt(
                    this->pre_master_secret_bytes.address(),
                    this->pre_master_secret_bytes.size(),
                    this->pre_master_secret_bytes.address(),
                    this->pre_master_secret_bytes.size(),
                    &result_length);
                if (!success)
                    return false;

                this->pre_master_secret_bytes.resize(result_length);

                PreMasterSecret pre_master_secret;

                std::shared_ptr<PreMasterSecretFrame> frame = std::make_shared<PreMasterSecretFrame>(&pre_master_secret);

				ElementSource<byte> element_source(this->pre_master_secret_bytes.address(), this->pre_master_secret_bytes.size());

				success = element_source.deliver_elements(frame.get());
				if (!success)
					return false;

                // rfc 5246 section 7.4.7.1:
                //Note: The version number in the PreMasterSecret is the version
                //offered by the client in the ClientHello.client_version, not the
                //version negotiated for the connection.  This feature is designed to
                //prevent rollback attacks.  Unfortunately, some old implementations
                //use the negotiated version instead, and therefore checking the
                //version number may lead to failure to interoperate with such
                //incorrect client implementations.

                //Client implementations MUST always send the correct version number in
                //PreMasterSecret.  If ClientHello.client_version is TLS 1.1 or higher,
                //server implementations MUST check the version number as described in
                //the note below.  If the version number is TLS 1.0 or earlier, server
                //implementations SHOULD check the version number, but MAY have a
                //configuration option to disable the check.  Note that if the check
                //fails, the PreMasterSecret SHOULD be randomized as described below.

                if (pre_master_secret.client_version != this->clientHello.client_version)
                    return Basic::globals->HandleError("Could be version roll-back attack.", 0);
            }
            break;

        // $$ implement DHE_DSS
        //case KeyExchangeAlgorithm::DHE_DSS:
        //    break;

        default:
            return Basic::globals->HandleError("ServerHandshake::ProcessClientKeyExchange unexpected key_exchange_algorithm", 0);
        }

        return true;
    }

    void ServerHandshake::PartitionKeyMaterial(ByteString* key_material)
    {
        Tls::globals->Partition(key_material, this->security_parameters->mac_key_length, &this->session->pending_read_state->MAC_key);
        Tls::globals->Partition(key_material, this->security_parameters->mac_key_length, &this->session->pending_write_state->MAC_key);
        Tls::globals->Partition(key_material, this->security_parameters->enc_key_length, &this->session->pending_read_state->encryption_key);
        Tls::globals->Partition(key_material, this->security_parameters->enc_key_length, &this->session->pending_write_state->encryption_key);

        if (this->session->version <= 0x0301)
        {
            Tls::globals->Partition(key_material, this->security_parameters->block_length, this->session->pending_read_state->IV.get());
            Tls::globals->Partition(key_material, this->security_parameters->block_length, this->session->pending_write_state->IV.get());
        }
    }
}