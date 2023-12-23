#include "stdafx.h"
#include "Tls.ServerHandshake.h"
#include "Basic.CountStream.h"
#include "Basic.HashAlgorithm.h"
#include "Basic.HashStream.h"
#include "Basic.Cng.h"
#include "Basic.FrameStream.h"
#include "Tls.RecordLayer.h"
#include "Tls.HandshakeFrame.h"
#include "Tls.CertificatesFrame.h"
#include "Tls.PreMasterSecretFrame.h"
#include "Tls.RandomFrame.h"
#include "Tls.Globals.h"
#include "Tls.ServerHelloFrame.h"
#include "Dynamo.Globals.h"
#include "Basic.ByteVector.h"

namespace Tls
{
	using namespace Basic;

	void ServerHandshake::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::start_state:
			Event::AddObserver<byte>(event, this->handshake_messages);

			this->handshake_frame.Initialize(&this->handshake);
			switch_to_state(State::expecting_client_hello_state);
			break;

		case State::expecting_client_hello_state:
			if (this->handshake_frame.Pending())
			{
				this->handshake_frame.Process(event, yield);
			}
			else if (this->handshake_frame.Failed())
			{
				switch_to_state(State::handshake_frame_1_failed);
			}
			else if (this->handshake.msg_type != HandshakeType::client_hello)
			{
				switch_to_state(State::expecting_client_hello_error);
			}
			else
			{
				this->client_hello_frame.Initialize(&this->clientHello, this->handshake.length);
				switch_to_state(State::hello_frame_pending_state);
			}
			break;

		case State::hello_frame_pending_state:
			if (this->client_hello_frame.Pending())
			{
				this->client_hello_frame.Process(event, yield);
			}
			else if (this->client_hello_frame.Failed())
			{
				switch_to_state(State::hello_frame_failed);
			}
			else
			{
				if (this->clientHello.client_version < this->session->version_low)
				{
					switch_to_state(State::client_version_error);
					return;
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
					return;
				}

				success = this->security_parameters->InitializeCipherSuite(this->session->version, cipher_suite, &this->key_exchange_algorithm);
				if (!success)
				{
					switch_to_state(State::InitializeCipherSuite_failed);
					return;
				}

				SignatureAndHashAlgorithms supported_signature_algorithms = this->clientHello.supported_signature_algorithms;
				if (supported_signature_algorithms.size() == 0)
				{
					// -  If the negotiated key exchange algorithm is one of (RSA, DHE_RSA,
					// 	DH_RSA, RSA_PSK, ECDH_RSA, ECDHE_RSA), behave as if client had
					// 	sent the value {sha1,rsa}.
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
					// 	DH_DSS), behave as if the client had sent the value {sha1,dsa}.
					else if (this->key_exchange_algorithm == KeyExchangeAlgorithm::DHE_DSS ||
						this->key_exchange_algorithm == KeyExchangeAlgorithm::DH_DSS)
					{
						SignatureAndHashAlgorithm alg = {sha1, dsa};
						supported_signature_algorithms.push_back(alg);
					}

					// -  If the negotiated key exchange algorithm is one of (ECDH_ECDSA,
					// 	ECDHE_ECDSA), behave as if the client had sent value {sha1,ecdsa}.
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
					Basic::globals->HandleError("ServerHandshake::Process BCryptGenRandom", error);
					switch_to_state(State::BCryptGenRandom_1_failed);
					return;
				}

				ServerHello serverHello;
				serverHello.cipher_suite = cipher_suite;
				serverHello.compression_method = CompressionMethod::cm_null;
				serverHello.server_version = this->session->version;
				serverHello.session_id = this->session->session_id;
				serverHello.random = this->security_parameters->server_random;

				Inline<ServerHelloFrame> server_hello_frame;
				server_hello_frame.Initialize(&serverHello, 0);

				success = WriteMessage(this->session, HandshakeType::server_hello, &server_hello_frame);
				if (!success)
				{
					switch_to_state(State::WriteMessage_1_failed);
					return;
				}

				switch(this->key_exchange_algorithm)
				{
				case KeyExchangeAlgorithm::_KEA_RSA:
					{
						CertificatesFrame::Ref frame;
						Dynamo::globals->CreateCertFrame(&frame);

						bool success = WriteMessage(this->session, HandshakeType::certificate, frame);
						if (!success)
						{
							switch_to_state(State::WriteMessage_4_failed);
							return;
						}
					}
					break;

				default:
					switch_to_state(State::unexpected_key_exchange_algorithm_2_error);
					return;
				}

				success = WriteMessage(this->session, HandshakeType::server_hello_done, 0);
				if (!success)
				{
					switch_to_state(State::WriteMessage_2_failed);
					return;
				}

				this->handshake_frame.Initialize(&this->handshake);
				switch_to_state(State::expecting_client_key_exchange_state);

				this->session->Flush();
			}
			break;

		case State::expecting_client_key_exchange_state:
			if (this->handshake_frame.Pending())
			{
				this->handshake_frame.Process(event, yield);
			}
			else if (this->handshake_frame.Failed())
			{
				switch_to_state(State::handshake_frame_2_failed);
			}
			else if (this->handshake.msg_type != HandshakeType::client_key_exchange)
			{
				switch_to_state(State::expecting_client_key_exchange_error);
			}
			else
			{
				switch(this->key_exchange_algorithm)
				{
				case KeyExchangeAlgorithm::_KEA_RSA:
					{
						this->pre_master_secret_bytes = New<ByteVector>();

						if (this->session->version == 0x0300)
							this->pre_master_secret_frame.Initialize(this->pre_master_secret_bytes, 48);
						else
							this->pre_master_secret_frame.Initialize(this->pre_master_secret_bytes);

						switch_to_state(State::pre_master_secret_frame_pending);
					}
					break;

				default:
					switch_to_state(State::unexpected_key_exchange_algorithm_1_error);
					break;
				}
			}
			break;

		case State::pre_master_secret_frame_pending:
			if (this->pre_master_secret_frame.Pending())
			{
				this->pre_master_secret_frame.Process(event, yield);
			}
			else if (this->pre_master_secret_frame.Failed())
			{
				switch_to_state(State::pre_master_secret_frame_failed);
			}
			else
			{
				ByteVector::Ref pre_master_secret_bytes = New<ByteVector>();

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

					this->pre_master_secret_bytes = New<ByteVector>();
					this->pre_master_secret_bytes->resize(48);

					NTSTATUS error = BCryptGenRandom(0, this->pre_master_secret_bytes->FirstElement(), this->pre_master_secret_bytes->size(), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
					if (error != 0)
					{
						Basic::globals->HandleError("ServerHandshake::Process BCryptGenRandom", error);
						switch_to_state(State::BCryptGenRandom_2_failed);
						return;
					}
				}

				CalculateKeys(this->pre_master_secret_bytes);

				this->pre_master_secret_bytes = 0;

				opaque label[] = { 'c', 'l', 'i', 'e', 'n', 't', ' ', 'f', 'i', 'n', 'i', 's', 'h', 'e', 'd', };

				this->finished_expected.resize(this->security_parameters->verify_data_length);

				CalculateVerifyData(label, sizeof(label), &finished_expected[0], finished_expected.size());

				switch_to_state(State::expecting_cipher_change_state);
			}
			break;

		case State::expecting_cipher_change_state:
			{
				if (event->get_type() != Tls::EventType::change_cipher_spec_event)
				{
					(*yield) = true;
					return;
				}

				this->handshake_frame.Initialize(&this->handshake);
				switch_to_state(State::expecting_finished_state);
			}
			break;

		case State::expecting_finished_state:
			if (this->handshake_frame.Pending())
			{
				this->handshake_frame.Process(event, yield);
			}
			else if (this->handshake_frame.Failed())
			{
				switch_to_state(State::handshake_frame_3_failed);
			}
			else if (this->handshake.msg_type != HandshakeType::finished)
			{
				switch_to_state(State::expecting_finished_error);
			}
			else if (this->handshake.length != this->security_parameters->verify_data_length)
			{
				switch_to_state(State::handshake_length_error);
			}
			else
			{
				this->finished_received.resize(this->security_parameters->verify_data_length);
				this->finished_received_frame.Initialize(&this->finished_received[0], this->finished_received.size());
				switch_to_state(State::finished_received_frame_pending_state);
			}
			break;

		case State::finished_received_frame_pending_state:
			if (this->finished_received_frame.Pending())
			{
				this->finished_received_frame.Process(event, yield);
			}
			else if (this->finished_received_frame.Failed())
			{
				switch_to_state(State::finished_received_frame_failed);
			}
			else
			{
				for (uint32 i = 0; i < this->security_parameters->verify_data_length; i++)
				{
					if (finished_received[i] != finished_expected[i])
					{
						switch_to_state(State::finished_received_error);
						return;
					}
				}

				opaque label[] = { 's', 'e', 'r', 'v', 'e', 'r', ' ', 'f', 'i', 'n', 'i', 's', 'h', 'e', 'd', };

				this->finished_sent = New<ByteVector>();
				this->finished_sent->resize(this->security_parameters->verify_data_length);

				Event::RemoveObserver<byte>(event, this->handshake_messages);

				CalculateVerifyData(label, sizeof(label), this->finished_sent->FirstElement(), this->finished_sent->size());

				ZeroMemory(this->handshake_messages->FirstElement(), this->handshake_messages->size());
				this->handshake_messages->resize(0);

				this->session->WriteChangeCipherSpec();

				bool success = WriteMessage(this->session, HandshakeType::finished, this->finished_sent);
				if (!success)
				{
					switch_to_state(State::WriteMessage_3_failed);
					return;
				}

				this->session->Flush();

				Dynamo::globals->DebugWriter()->WriteFormat<0x100>("TLS server handshake successfully negotiated 0x%04X", this->session->version);
				Dynamo::globals->DebugWriter()->WriteLine();

				// $ handle renegotiates, etc.
				switch_to_state(State::done_state);
			}
			break;

		default:
			throw new Exception("Tls::ServerHandshake::Process unexpected state");
		}
	}

	bool ServerHandshake::ProcessClientKeyExchange(KeyExchangeAlgorithm key_exchange_algorithm)
	{
		switch(key_exchange_algorithm)
		{
		case KeyExchangeAlgorithm::_KEA_RSA:
			{
				DWORD result_length = 0;

				bool success = Dynamo::globals->CertDecrypt(
					this->pre_master_secret_bytes->FirstElement(),
					this->pre_master_secret_bytes->size(),
					this->pre_master_secret_bytes->FirstElement(),
					this->pre_master_secret_bytes->size(),
					&result_length);
				if (!success)
					return false;

				this->pre_master_secret_bytes->resize(result_length);

				PreMasterSecret pre_master_secret;

				Inline<PreMasterSecretFrame> frame;
				frame.Initialize(&pre_master_secret);

				success = FrameStream<byte>::Process(&frame, this->pre_master_secret_bytes->FirstElement(), this->pre_master_secret_bytes->size());
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

		default:
			return Basic::globals->HandleError("ServerHandshake::ProcessClientKeyExchange unexpected key_exchange_algorithm", 0);
		}

		return true;
	}

	void ServerHandshake::PartitionKeyMaterial(std::vector<opaque>* key_material)
	{
		Tls::globals->Partition(key_material, this->security_parameters->mac_key_length, &this->session->pending_read_state->MAC_key);
		Tls::globals->Partition(key_material, this->security_parameters->mac_key_length, &this->session->pending_write_state->MAC_key);
		Tls::globals->Partition(key_material, this->security_parameters->enc_key_length, &this->session->pending_read_state->encryption_key);
		Tls::globals->Partition(key_material, this->security_parameters->enc_key_length, &this->session->pending_write_state->encryption_key);

		if (this->session->version <= 0x0301)
		{
			Tls::globals->Partition(key_material, this->security_parameters->block_length, this->session->pending_read_state->IV);
			Tls::globals->Partition(key_material, this->security_parameters->block_length, this->session->pending_write_state->IV);
		}
	}
}
