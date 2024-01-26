// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Globals.h"
#include "Tls.ConnectionState.h"
#include "Tls.RecordFrame.h"
#include "Tls.Globals.h"

namespace Tls
{
    ConnectionState::ConnectionState() :
        sequence_number(0)
    {
    }

    ConnectionState::~ConnectionState()
    {
    }

    bool ConnectionState::Initialize(std::shared_ptr<SecurityParameters> security_parameters)
    {
        wchar_t* algorithm;

        switch(security_parameters->bulk_cipher_algorithm)
        {
        case BulkCipherAlgorithm::bca_null:
            algorithm = 0;
            break;

        case BulkCipherAlgorithm::aes:
            algorithm = BCRYPT_AES_ALGORITHM;
            break;

        case BulkCipherAlgorithm::rc4:
            algorithm = BCRYPT_RC4_ALGORITHM;
            break;

        case BulkCipherAlgorithm::_3des:
            algorithm = BCRYPT_3DES_ALGORITHM;
            break;

        default:
            // $ perhaps should be LogDebug - don't want to be open to a logging attack, but then again do want to know if otherwise valid TLS
            // connections are failing due to an unsupported algorithm
            Basic::LogError("Tls", "ConnectionState", "Initialize", "switch(security_parameters->bulk_cipher_algorithm) default: unsupported cipher_suite", security_parameters->bulk_cipher_algorithm);
            return false;
        }

        if (algorithm != 0)
        {
            Basic::BCRYPT_ALG_HANDLE bulk_cipher_algorithm_handle;

            NTSTATUS error = BCryptOpenAlgorithmProvider(&bulk_cipher_algorithm_handle, algorithm, 0, 0);
            if (error != 0)
            {
                Basic::LogError("Tls", "ConnectionState", "Initialize", "BCryptOpenAlgorithmProvider", error);
                return false;
            }

            ByteString key_blob;
            key_blob.reserve(sizeof(BCRYPT_KEY_DATA_BLOB_HEADER) + this->encryption_key.size());
            key_blob.resize(sizeof(BCRYPT_KEY_DATA_BLOB_HEADER));
            key_blob.insert(key_blob.end(), this->encryption_key.begin(), this->encryption_key.end());

            BCRYPT_KEY_DATA_BLOB_HEADER* header = (BCRYPT_KEY_DATA_BLOB_HEADER*)key_blob.address();
            header->dwMagic = BCRYPT_KEY_DATA_BLOB_MAGIC;
            header->dwVersion = BCRYPT_KEY_DATA_BLOB_VERSION1;
            header->cbKeyData = this->encryption_key.size();

            error = BCryptImportKey(bulk_cipher_algorithm_handle, 0, BCRYPT_KEY_DATA_BLOB, &this->key_handle, 0, 0, key_blob.address(), key_blob.size(), 0);
            if (error != 0)
            {
                Basic::LogError("Tls", "ConnectionState", "Initialize", "BCryptImportKey", error);
                return false;
            }
        }

        this->security_parameters = security_parameters;

        return true;
    }

    void ConnectionState::MAC(Record* compressed, byte* output, uint8 output_max)
    {
        Serializer<uint64> sequence_number_serializer(&this->sequence_number);
        Serializer<Record> compressed_serializer(compressed);

        IStreamWriter<byte>* seed[] = { &sequence_number_serializer, &compressed_serializer, };

        Tls::globals->HMAC_hash(
            this->security_parameters->mac_algorithm,
            &this->MAC_key,
            seed,
            _countof(seed),
            output,
            output_max);
    }
}