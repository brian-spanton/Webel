#include "stdafx.h"
#include "Tls.ConnectionState.h"
#include "Tls.NumberFrame.h"
#include "Tls.RecordFrame.h"
#include "Tls.Globals.h"

namespace Tls
{
	ConnectionState::ConnectionState() :
		encryption_key(0),
		sequence_number(0)
	{
	}

	ConnectionState::~ConnectionState()
	{
	}

	bool ConnectionState::Initialize(SecurityParameters* security_parameters)
	{
		switch(security_parameters->bulk_cipher_algorithm)
		{
		case BulkCipherAlgorithm::bca_null:
			break;

		case BulkCipherAlgorithm::aes:
			{
				Basic::BCRYPT_ALG_HANDLE bulk_cipher_algorithm_handle;

				NTSTATUS error = BCryptOpenAlgorithmProvider(&bulk_cipher_algorithm_handle, BCRYPT_AES_ALGORITHM, 0, 0);
				if (error != 0)
					throw new Exception("BCryptOpenAlgorithmProvider", error);

				std::vector<byte> key_blob;
				key_blob.reserve(sizeof(BCRYPT_KEY_DATA_BLOB_HEADER) + this->encryption_key.size());
				key_blob.resize(sizeof(BCRYPT_KEY_DATA_BLOB_HEADER));
				key_blob.insert(key_blob.end(), this->encryption_key.begin(), this->encryption_key.end());

				BCRYPT_KEY_DATA_BLOB_HEADER* header = (BCRYPT_KEY_DATA_BLOB_HEADER*)&key_blob[0];
				header->dwMagic = BCRYPT_KEY_DATA_BLOB_MAGIC;
				header->dwVersion = BCRYPT_KEY_DATA_BLOB_VERSION1;
				header->cbKeyData = this->encryption_key.size();

				error = BCryptImportKey(bulk_cipher_algorithm_handle, 0, BCRYPT_KEY_DATA_BLOB, &this->key_handle, 0, 0, &key_blob[0], key_blob.size(), 0);
				if (error != 0)
					return Basic::globals->HandleError("BCryptImportKey", error);
			}
			break;

		default:
			return Basic::globals->HandleError("Tls::ConnectionState::Initialize unsupported cipher_suite", 0);
		}

		this->security_parameters = security_parameters;

		return true;
	}

	void ConnectionState::MAC(Record* compressed, opaque* output, uint8 output_max)
	{
		NumberFrame<uint64>::Ref seqFrame = New<NumberFrame<uint64> >();
		seqFrame->Initialize(&this->sequence_number);

		RecordFrame::Ref compressedFrame = New<RecordFrame>();
		compressedFrame->Initialize(compressed);

		ISerializable* seed[] = { seqFrame, compressedFrame, };

		Tls::globals->HMAC_hash(
			this->security_parameters->mac_algorithm,
			&this->MAC_key[0],
			this->MAC_key.size(),
			seed,
			_countof(seed),
			output,
			output_max);
	}
}