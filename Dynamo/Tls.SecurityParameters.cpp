#include "stdafx.h"
#include "Tls.SecurityParameters.h"
#include "Tls.Globals.h"
#include "Dynamo.Globals.h"

namespace Tls
{
	SecurityParameters::SecurityParameters() :
		prf_algorithm(tls_prf_sha256),
		bulk_cipher_algorithm(bca_null),
		cipher_type(stream),
		enc_key_length(0),
		block_length(0),
		mac_algorithm(ma_null),
		mac_length(0),
		mac_key_length(0),
		compression_algorithm(cm_null)
	{
		ZeroMemory(master_secret, sizeof(master_secret));
		ZeroMemory(&client_random, sizeof(client_random));
		ZeroMemory(&server_random, sizeof(server_random));
	}

	SecurityParameters::~SecurityParameters()
	{
	}

	bool SecurityParameters::InitializeCipherSuite(ProtocolVersion version, CipherSuite cipher_suite, KeyExchangeAlgorithm* key_exchange_algorithm)
	{
		InterimCipher interim_cipher;

		switch (cipher_suite)
		{
		case CipherSuite::cs_TLS_RSA_WITH_AES_128_CBC_SHA:
			(*key_exchange_algorithm) = KeyExchangeAlgorithm::_KEA_RSA;
			interim_cipher = InterimCipher::ic_AES_128_CBC;
			this->mac_algorithm = MACAlgorithm::hmac_sha1;
			break;

		case CipherSuite::cs_TLS_RSA_WITH_3DES_EDE_CBC_SHA:
			(*key_exchange_algorithm) = KeyExchangeAlgorithm::_KEA_RSA;
			interim_cipher = InterimCipher::ic_3DES_EDE_CBC;
			this->mac_algorithm = MACAlgorithm::hmac_sha1;
			break;

		case CipherSuite::cs_TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA:
			(*key_exchange_algorithm) = KeyExchangeAlgorithm::DHE_DSS;
			interim_cipher = InterimCipher::ic_3DES_EDE_CBC;
			this->mac_algorithm = MACAlgorithm::hmac_sha1;
			break;

		default:
			return Basic::globals->HandleError("Tls::SecurityParameters::Initialize unsupported cipher_suite", 0);
		}

		switch (version)
		{
		case 0x0301:
			this->prf_algorithm = PRFAlgorithm::tls_prf_tls_v1;
			break;

		case 0x0302:
			this->prf_algorithm = PRFAlgorithm::tls_prf_tls_v1;
			break;

		case 0x0303:
			this->prf_algorithm = PRFAlgorithm::tls_prf_sha256;
			break;

		default:
			return Basic::globals->HandleError("Tls::SecurityParameters::Initialize unsupported version", 0);
		}

		//                        Key      IV   Block
		// Cipher        Type    Material  Size  Size
		// ------------  ------  --------  ----  -----
		// NULL          Stream      0       0    N/A
		// RC4_128       Stream     16       0    N/A
		// 3DES_EDE_CBC  Block      24       8      8
		// AES_128_CBC   Block      16      16     16
		// AES_256_CBC   Block      32      16     16

		switch(interim_cipher)
		{
		case InterimCipher::ic_NULL:
			this->bulk_cipher_algorithm = BulkCipherAlgorithm::bca_null;
			this->cipher_type = CipherType::stream;
			this->enc_key_length = 0;
			this->block_length = 0;
			break;

		case InterimCipher::ic_RC4_128:
			this->bulk_cipher_algorithm = BulkCipherAlgorithm::rc4;
			this->cipher_type = CipherType::stream;
			this->enc_key_length = 16;
			this->block_length = 0;
			break;

		case InterimCipher::ic_3DES_EDE_CBC:
			this->bulk_cipher_algorithm = BulkCipherAlgorithm::_3des;
			this->cipher_type = CipherType::block;
			this->enc_key_length = 24;
			this->block_length = 8;
			break;

		case InterimCipher::ic_AES_128_CBC:
			this->bulk_cipher_algorithm = BulkCipherAlgorithm::aes;
			this->cipher_type = CipherType::block;
			this->enc_key_length = 16;
			this->block_length = 16;
			break;

		case InterimCipher::ic_AES_256_CBC:
			this->bulk_cipher_algorithm = BulkCipherAlgorithm::aes;
			this->cipher_type = CipherType::block;
			this->enc_key_length = 32;
			this->block_length = 16;
			break;

		default:
			return Basic::globals->HandleError("Tls::SecurityParameters::Initialize unsupported interim_cipher", 0);
		}

		//rfc 5246 section 7.4.9:
		//In previous versions of TLS, the verify_data was always 12 octets
		//long.  In the current version of TLS, it depends on the cipher
		//suite.  Any cipher suite which does not explicitly specify
		//verify_data_length has a verify_data_length equal to 12.  This
		//includes all existing cipher suites.  Note that this
		//representation has the same encoding as with previous versions.
		//Future cipher suites MAY specify other lengths but such length
		//MUST be at least 12 bytes.
		this->verify_data_length = 12;

		// MAC       Algorithm    mac_length  mac_key_length
		// --------  -----------  ----------  --------------
		// NULL      N/A              0             0
		// MD5       HMAC-MD5        16            16
		// SHA       HMAC-SHA1       20            20
		// SHA256    HMAC-SHA256     32            32

		switch(this->mac_algorithm)
		{
		case MACAlgorithm::ma_null:
			this->mac_length = 0;
			this->mac_key_length = 0;
			break;

		case MACAlgorithm::hmac_md5:
			this->mac_length = 16;
			this->mac_key_length = 16;
			break;

		case MACAlgorithm::hmac_sha1:
			this->mac_length = 20;
			this->mac_key_length = 20;
			break;

		case MACAlgorithm::hmac_sha256:
			this->mac_length = 32;
			this->mac_key_length = 32;
			break;

		default:
			return Basic::globals->HandleError("Tls::SecurityParameters::Initialize unsupported mac_algorithm", 0);
		}

		return true;
	}
}