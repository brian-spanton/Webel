#include "stdafx.h"
#include "Tls.ClientHandshake.h"
#include "Dynamo.Globals.h"
#include "Basic.CountStream.h"
#include "Basic.HashAlgorithm.h"
#include "Basic.HashStream.h"
#include "Tls.ClientHelloFrame.h"
#include "Tls.RecordLayer.h"
#include "Tls.HandshakeFrame.h"
#include "Tls.PreMasterSecretFrame.h"
#include "Tls.RandomFrame.h"
#include "Tls.Globals.h"

namespace Tls
{
	using namespace Basic;

	void ClientHandshake::Initialize(RecordLayer* session)
	{
		__super::Initialize(session);
		this->certificates_frame.Initialize(&this->certificates);
	}

	void ClientHandshake::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::start_state:
			{
				Event::AddObserver<byte>(event, this->handshake_messages);

				this->security_parameters->client_random.gmt_unix_time = 0;

				NTSTATUS error = BCryptGenRandom(0, this->security_parameters->client_random.random_bytes, sizeof(this->security_parameters->client_random.random_bytes), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
				if (error != 0)
					throw new Exception("ClientHandshake::ProcessConnect BCryptGenRandom", error);

				ClientHello clientHello;
				clientHello.client_version = this->session->version_high;
				clientHello.random = this->security_parameters->client_random;
				clientHello.cipher_suites.push_back(CipherSuite::cs_TLS_RSA_WITH_AES_128_CBC_SHA);
				clientHello.compression_methods.push_back(CompressionMethod::cm_null);

				Inline<ClientHelloFrame> client_hello_frame;
				client_hello_frame.Initialize(&clientHello, 0);

				bool success = WriteMessage(this->session, HandshakeType::client_hello, &client_hello_frame);
				if (!success)
				{
					switch_to_state(State::WriteMessage_1_failed);
					return;
				}

				this->session->Flush();

				this->handshake_frame.Initialize(&this->handshake);
				switch_to_state(State::expecting_server_hello_state);
			}
			break;

		case State::expecting_server_hello_state:
			if (this->handshake_frame.Pending())
			{
				this->handshake_frame.Process(event, yield);
			}
			else if (this->handshake_frame.Failed())
			{
				switch_to_state(State::handshake_frame_1_failed);
			}
			else if (this->handshake.msg_type != HandshakeType::server_hello)
			{
				switch_to_state(State::expecting_server_hello_error);
			}
			else
			{
				this->server_hello_frame.Initialize(&this->serverHello, this->handshake.length);
				switch_to_state(State::server_hello_frame_pending_state);
			}
			break;

		case State::server_hello_frame_pending_state:
			if (this->server_hello_frame.Pending())
			{
				this->server_hello_frame.Process(event, yield);
			}
			else if (this->server_hello_frame.Failed())
			{
				switch_to_state(State::server_hello_frame_failed);
			}
			else
			{
				if (this->serverHello.server_version < this->session->version_low || this->serverHello.server_version > this->session->version_high)
				{
					switch_to_state(State::server_version_error);
					return;
				}

				this->session->FinalizeVersion(this->serverHello.server_version);

				this->security_parameters->server_random = this->serverHello.random;

				bool success = this->security_parameters->InitializeCipherSuite(this->session->version, this->serverHello.cipher_suite, &this->key_exchange_algorithm);
				if (!success)
				{
					switch_to_state(State::InitializeCipherSuite_failed);
					return;
				}

				switch(this->key_exchange_algorithm)
				{
				case KeyExchangeAlgorithm::_KEA_RSA:
					this->handshake_frame.Initialize(&this->handshake);
					switch_to_state(State::expecting_certificate_state);
					break;

				default:
					switch_to_state(State::unexpected_key_exchange_algorithm_1_error);
					return;
				}
			}
			break;

		case State::expecting_certificate_state:
			if (this->handshake_frame.Pending())
			{
				this->handshake_frame.Process(event, yield);
			}
			else if (this->handshake_frame.Failed())
			{
				switch_to_state(State::handshake_frame_2_failed);
			}
			else if (this->handshake.msg_type != HandshakeType::certificate)
			{
				switch_to_state(State::expecting_certificate_error);
			}
			else
			{
				switch_to_state(State::certificates_frame_pending_state);
			}
			break;

		case State::certificates_frame_pending_state:
			if (this->certificates_frame.Pending())
			{
				this->certificates_frame.Process(event, yield);
			}
			else if (this->certificates_frame.Failed())
			{
				switch_to_state(State::certificates_frame_failed);
			}
			else
			{
				this->handshake_frame.Initialize(&this->handshake);
				switch_to_state(State::expecting_server_hello_done_state);
			}
			break;

		case State::expecting_server_hello_done_state:
			if (this->handshake_frame.Pending())
			{
				this->handshake_frame.Process(event, yield);
			}
			else if (this->handshake_frame.Failed())
			{
				switch_to_state(State::handshake_frame_3_failed);
			}
			else if (this->handshake.msg_type != HandshakeType::server_hello_done)
			{
				switch_to_state(State::expecting_server_hello_done_error);
			}
			else if (this->handshake.length != 0)
			{
				switch_to_state(State::handshake_length_1_error);
			}
			else
			{
				Basic::PCCERT_CONTEXT cert = CertCreateCertificateContext(X509_ASN_ENCODING, &(this->certificates[0][0]), this->certificates[0].size());
				if (cert == 0)
				{
					switch_to_state(State::CertCreateCertificateContext_failed);
					return;
				}

				Basic::BCRYPT_KEY_HANDLE public_key;
				bool success = (bool)CryptImportPublicKeyInfoEx2(X509_ASN_ENCODING, &cert->pCertInfo->SubjectPublicKeyInfo, 0, 0, &public_key);
				if (!success)
				{
					switch_to_state(State::CryptImportPublicKeyInfoEx2_failed);
					return;
				}

				PreMasterSecret pre_master_secret;
				pre_master_secret.client_version = this->session->version_high;

				NTSTATUS error = BCryptGenRandom(0, pre_master_secret.random, sizeof(pre_master_secret.random), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
				if (error != 0)
				{
					Basic::globals->HandleError("Tls::ClientHandshake::Process BCryptGenRandom failed", error);
					switch_to_state(State::BCryptGenRandom_failed);
					return;
				}

				this->pre_master_secret_bytes = New<ByteVector>();

				Inline<PreMasterSecretFrame> pre_master_secret_frame;
				pre_master_secret_frame.Initialize(&pre_master_secret);

				pre_master_secret_frame.SerializeTo(this->pre_master_secret_bytes);

				// don't keep the pre_paster_secret in memory longer than necessary
				ZeroMemory(&pre_master_secret, sizeof(pre_master_secret));

				DWORD result_length = 0;

				error = BCryptEncrypt(
					public_key,
					this->pre_master_secret_bytes->FirstElement(),
					this->pre_master_secret_bytes->size(),
					0,
					0,
					0,
					0,
					0,
					&result_length,
					BCRYPT_PAD_PKCS1);
				if (error != 0)
				{
					Basic::globals->HandleError("ClientHandshake::Process BCryptEncrypt", error);
					switch_to_state(State::BCryptEncrypt_1_failed);
					return;
				}

				std::vector<opaque> pre_master_secret_encrypted;
				pre_master_secret_encrypted.resize(result_length);

				error = BCryptEncrypt(
					public_key,
					this->pre_master_secret_bytes->FirstElement(),
					this->pre_master_secret_bytes->size(),
					0,
					0,
					0,
					&pre_master_secret_encrypted[0],
					pre_master_secret_encrypted.size(),
					&result_length,
					BCRYPT_PAD_PKCS1);
				if (error != 0)
				{
					Basic::globals->HandleError("ClientHandshake::Process BCryptEncrypt", error);
					switch_to_state(State::BCryptEncrypt_2_failed);
					return;
				}

				pre_master_secret_encrypted.resize(result_length);

				CalculateKeys(this->pre_master_secret_bytes);

				this->pre_master_secret_bytes = 0;

				EncryptedPreMasterSecretFrame::Ref encryptedFrame = New<EncryptedPreMasterSecretFrame>();
				if (this->session->version == 0x0300)
					encryptedFrame->Initialize(&pre_master_secret_encrypted, 48);
				else
					encryptedFrame->Initialize(&pre_master_secret_encrypted);

				success = WriteMessage(this->session, HandshakeType::client_key_exchange, encryptedFrame.item());
				if (!success)
				{
					switch_to_state(State::WriteMessage_2_failed);
					return;
				}

				opaque clientFinishedLabel[] = { 'c', 'l', 'i', 'e', 'n', 't', ' ', 'f', 'i', 'n', 'i', 's', 'h', 'e', 'd', };

				this->finished_sent = New<ByteVector>();
				this->finished_sent->resize(this->security_parameters->verify_data_length);

				CalculateVerifyData(clientFinishedLabel, sizeof(clientFinishedLabel), this->finished_sent->FirstElement(), this->finished_sent->size());

				this->session->WriteChangeCipherSpec();

				success = WriteMessage(this->session, HandshakeType::finished, this->finished_sent);
				if (!success)
				{
					switch_to_state(State::WriteMessage_3_failed);
					return;
				}

				opaque serverFinishedLabel[] = { 's', 'e', 'r', 'v', 'e', 'r', ' ', 'f', 'i', 'n', 'i', 's', 'h', 'e', 'd', };

				this->finished_expected.resize(this->security_parameters->verify_data_length);

				Event::RemoveObserver<byte>(event, this->handshake_messages);

				CalculateVerifyData(serverFinishedLabel, sizeof(serverFinishedLabel), &finished_expected[0], finished_expected.size());

				ZeroMemory(this->handshake_messages->FirstElement(), this->handshake_messages->size());
				this->handshake_messages->resize(0);

				switch_to_state(State::expecting_cipher_change_state);

				this->session->Flush();
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
				switch_to_state(State::handshake_frame_4_failed);
			}
			else if (this->handshake.msg_type != HandshakeType::finished)
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

				Dynamo::globals->DebugWriter()->WriteFormat<0x100>("TLS client handshake successfully negotiated 0x%04X", this->session->version);
				Dynamo::globals->DebugWriter()->WriteLine();

				// $ handle renegotiates, etc.
				switch_to_state(State::done_state);
			}
			break;

		default:
			throw new Exception("Tls::ClientHandshake::Process unexpected state");
		}
	}

	void ClientHandshake::PartitionKeyMaterial(std::vector<opaque>* key_material)
	{
		Tls::globals->Partition(key_material, this->security_parameters->mac_key_length, &this->session->pending_write_state->MAC_key);
		Tls::globals->Partition(key_material, this->security_parameters->mac_key_length, &this->session->pending_read_state->MAC_key);
		Tls::globals->Partition(key_material, this->security_parameters->enc_key_length, &this->session->pending_write_state->encryption_key);
		Tls::globals->Partition(key_material, this->security_parameters->enc_key_length, &this->session->pending_read_state->encryption_key);

		if (this->session->version <= 0x0301)
		{
			Tls::globals->Partition(key_material, this->security_parameters->block_length, this->session->pending_write_state->IV);
			Tls::globals->Partition(key_material, this->security_parameters->block_length, this->session->pending_read_state->IV);
		}
	}
}
