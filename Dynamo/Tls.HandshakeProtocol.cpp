#include "stdafx.h"
#include "Tls.HandshakeProtocol.h"
#include "Basic.CountStream.h"
#include "Basic.HashAlgorithm.h"
#include "Basic.HashStream.h"
#include "Basic.Cng.h"
#include "Tls.ClientHelloFrame.h"
#include "Tls.ServerHelloFrame.h"
#include "Tls.RecordLayer.h"
#include "Tls.HandshakeFrame.h"
#include "Tls.CertificatesFrame.h"
#include "Tls.PreMasterSecretFrame.h"
#include "Tls.RandomFrame.h"
#include "Tls.Globals.h"
#include "Tls.RecordLayer.h"

namespace Tls
{
	using namespace Basic;

	void HandshakeProtocol::Initialize(RecordLayer* session)
	{
		__super::Initialize();

		this->security_parameters = New<SecurityParameters>();
		this->handshake_messages = New<ByteVector>();
		this->handshake_messages->reserve(0x1000);

		this->session = session;
	}

	void HandshakeProtocol::switch_to_state(uint32 state)
	{
		__super::switch_to_state(state);

		if (Succeeded())
		{
			this->session->ConnectApplication();
		}
		else if (Failed())
		{
			this->session->WriteEOF();
		}
	}

	void HandshakeProtocol::CalculateVerifyData(opaque* label, uint16 label_length, opaque* output, uint16 output_max)
	{
		if (output_max < this->security_parameters->verify_data_length)
			throw new Exception("Tls::HandshakeProtocol::CalculateVerifyData output_max < this->security_parameters->verify_data_length", 0);

		ISerializable* seed[] = { handshake_messages, };

		Basic::Ref<Basic::HashAlgorithm> hashAlgorithm = New<Basic::HashAlgorithm>();
		hashAlgorithm->Initialize(BCRYPT_MD5_ALGORITHM, false);

		ByteVector::Ref md5 = New<ByteVector>();
		md5->resize(hashAlgorithm->hash_output_length);
		Tls::globals->Hash(hashAlgorithm, 0, 0, seed, _countof(seed), md5->FirstElement(), md5->size());

		hashAlgorithm = New<Basic::HashAlgorithm>();
		hashAlgorithm->Initialize(BCRYPT_SHA1_ALGORITHM, false);

		ByteVector::Ref sha1 = New<ByteVector>();
		sha1->resize(hashAlgorithm->hash_output_length);
		Tls::globals->Hash(hashAlgorithm, 0, 0, seed, _countof(seed), sha1->FirstElement(), sha1->size());

		ISerializable* prf_seed[] = { md5, sha1, };

		Tls::globals->PRF(
			this->security_parameters->prf_algorithm,
			this->security_parameters->master_secret,
			sizeof(this->security_parameters->master_secret),
			label,
			label_length,
			prf_seed,
			_countof(prf_seed),
			output,
			this->security_parameters->verify_data_length);
	}

	bool HandshakeProtocol::WriteFinished(opaque* label, uint16 label_length)
	{
		// http://www.ietf.org/mail-archive/web/tls/current/msg09221.html

		ByteVector::Ref finished_data = New<ByteVector>();
		finished_data->resize(this->security_parameters->verify_data_length);

		CalculateVerifyData(label, label_length, finished_data->FirstElement(), finished_data->size());

		this->session->WriteChangeCipherSpec();

		bool success = WriteMessage(this->session, HandshakeType::finished, finished_data);
		if (!success)
			return false;

		return true;
	}

	bool HandshakeProtocol::WriteMessage(IStream<byte>* stream, HandshakeType msg_type, ISerializable* frame)
	{
		Handshake handshake;
		handshake.msg_type = msg_type;
		handshake.length = 0;

		if (frame != 0)
		{
			Basic::Ref<CountStream<byte> > count_stream = New<CountStream<byte> >();
			frame->SerializeTo(count_stream);
			handshake.length = count_stream->count;
		}

		Inline<HandshakeFrame> handshakeFrame;
		handshakeFrame.Initialize(&handshake);

		handshakeFrame.SerializeTo(this->handshake_messages);
		handshakeFrame.SerializeTo(stream);

		if (frame != 0)
		{
			frame->SerializeTo(this->handshake_messages);
			frame->SerializeTo(stream);
		}

		return true;
	}

	void HandshakeProtocol::CalculateKeys(ByteVector* pre_master_secret)
	{
		opaque masterSecretLabel[] = { 'm', 'a', 's', 't', 'e', 'r', ' ', 's', 'e', 'c', 'r', 'e', 't', };

		RandomFrame::Ref clientRandomFrame = New<RandomFrame>();
		clientRandomFrame->Initialize(&this->security_parameters->client_random);

		RandomFrame::Ref serverRandomFrame = New<RandomFrame>();
		serverRandomFrame->Initialize(&this->security_parameters->server_random);

		ISerializable* masterSecretSeed[] = { clientRandomFrame, serverRandomFrame, };

		Tls::globals->PRF(
			this->security_parameters->prf_algorithm,
			pre_master_secret->FirstElement(),
			pre_master_secret->size(),
			masterSecretLabel,
			sizeof(masterSecretLabel),
			masterSecretSeed,
			_countof(masterSecretSeed),
			this->security_parameters->master_secret,
			sizeof(this->security_parameters->master_secret));

		// generate keys per TLS section 6.3

		opaque keyExpansionLabel[] = { 'k', 'e', 'y', ' ', 'e', 'x', 'p', 'a', 'n', 's', 'i', 'o', 'n', };

		uint16 key_material_length = 2 * (this->security_parameters->mac_key_length + this->security_parameters->enc_key_length + this->security_parameters->block_length);
		std::vector<opaque> key_material;
		key_material.resize(key_material_length);

		ISerializable* keyExpansionSeed[] = { serverRandomFrame, clientRandomFrame, };

		Tls::globals->PRF(
			this->security_parameters->prf_algorithm,
			this->security_parameters->master_secret,
			sizeof(this->security_parameters->master_secret),
			keyExpansionLabel,
			sizeof(keyExpansionLabel),
			keyExpansionSeed,
			_countof(keyExpansionSeed),
			&key_material[0],
			key_material.size());

		this->session->pending_read_state->IV = New<ByteVector>();
		this->session->pending_write_state->IV = New<ByteVector>();

		PartitionKeyMaterial(&key_material);

		this->session->pending_read_state->Initialize(this->security_parameters);
		this->session->pending_write_state->Initialize(this->security_parameters);

		ZeroMemory(pre_master_secret->FirstElement(), pre_master_secret->size());
	}
}
