// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.HashAlgorithm.h"
#include "Basic.HashStream.h"
#include "Tls.Globals.h"
#include "Tls.SecurityParameters.h"

namespace Tls
{
	Globals* globals = 0;

	void Globals::Initialize()
	{
		// in order of preference
		this->supported_cipher_suites.push_back(CipherSuite::cs_TLS_RSA_WITH_AES_256_CBC_SHA256);
		this->supported_cipher_suites.push_back(CipherSuite::cs_TLS_RSA_WITH_AES_256_CBC_SHA);
		this->supported_cipher_suites.push_back(CipherSuite::cs_TLS_RSA_WITH_AES_128_CBC_SHA256);
		this->supported_cipher_suites.push_back(CipherSuite::cs_TLS_RSA_WITH_AES_128_CBC_SHA);
		this->supported_cipher_suites.push_back(CipherSuite::cs_TLS_RSA_WITH_3DES_EDE_CBC_SHA);
		//this->supported_cipher_suites.push_back(CipherSuite::cs_TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA); // $$
		this->supported_cipher_suites.push_back(CipherSuite::cs_TLS_RSA_WITH_RC4_128_MD5);
	}

	void Globals::PRF(PRFAlgorithm prf_algorithm, opaque* secret, uint32 secret_length, opaque* label, uint32 label_length, ISerializable** seed, uint32 seed_count, opaque* output, uint32 output_length)
	{
		std::vector<ISerializable*> currentSeed;

		MemoryRange::Ref labelFrame = New<MemoryRange>();
		labelFrame->Initialize(label, label_length);

		currentSeed.insert(currentSeed.end(), labelFrame);
		currentSeed.insert(currentSeed.end(), seed, seed + seed_count);

		PRF_hash(prf_algorithm, secret, secret_length, &currentSeed[0], currentSeed.size(), output, output_length);
	}

	void Globals::PRF_hash(PRFAlgorithm prf_algorithm, opaque* secret, uint32 secret_length, ISerializable** seed, uint32 seed_count, opaque* output, uint32 output_length)
	{
		switch(prf_algorithm)
		{
		case PRFAlgorithm::tls_prf_tls_v1:
			{
				uint32 S_length = (secret_length + 1) / 2;
				opaque* S1 = secret;
				opaque* S2 = secret + secret_length - S_length;

				std::vector<opaque> md5_stream;
				md5_stream.resize(2 * output_length);
				P_hash(BCRYPT_MD5_ALGORITHM, S1, S_length, seed, seed_count, &md5_stream[0], output_length, md5_stream.size());

				md5_stream.resize(output_length);

				std::vector<opaque> sha1_stream;
				sha1_stream.resize(2 * output_length);
				P_hash(BCRYPT_SHA1_ALGORITHM, S2, S_length, seed, seed_count, &sha1_stream[0], output_length, sha1_stream.size());

				sha1_stream.resize(output_length);

				for (uint32 i = 0; i < output_length; i++)
					output[i] = md5_stream[i] ^ sha1_stream[i];
			}
			break;

		default:
			throw new Exception("Tls::PRF unsupported PRFAlgorithm", 0);
		}
	}

	void Globals::P_hash(LPCWSTR algorithm, opaque* secret, uint32 secret_length, ISerializable** seed, uint32 seed_count, opaque* output, uint32 output_min, uint32 output_max)
	{
		Basic::HashAlgorithm::Ref hashAlgorithm = New<Basic::HashAlgorithm>();
		hashAlgorithm->Initialize(algorithm, true);

		std::vector<ISerializable*> a_minus_one;
		a_minus_one.insert(a_minus_one.end(), seed, seed + seed_count);

		// make sure this is outside the loop so it doesn't get deleted before it's used
		ByteVector::Ref a_current = New<ByteVector>();

		for(uint32 generated_length = 0; generated_length < output_min; generated_length += hashAlgorithm->hash_output_length)
		{
			a_current->resize(hashAlgorithm->hash_output_length);

			Hash(hashAlgorithm.item(), secret, secret_length, &a_minus_one[0], a_minus_one.size(), a_current->FirstElement(), a_current->size());

			a_minus_one.clear();
			a_minus_one.push_back(a_current);

			std::vector<ISerializable*> p_seed;
			p_seed.insert(p_seed.end(), a_current);
			p_seed.insert(p_seed.end(), seed, seed + seed_count);

			Hash(hashAlgorithm, secret, secret_length, &p_seed[0], p_seed.size(), output + generated_length, output_max - generated_length);
		}
	}

	void Globals::HMAC_hash(MACAlgorithm mac_algorithm, opaque* secret, uint32 secret_length, ISerializable** seed, uint32 seed_count, opaque* output, uint32 expected_output_length)
	{
		LPCWSTR cng_algorithm;

		switch(mac_algorithm)
		{
		case MACAlgorithm::hmac_sha1:
			cng_algorithm = BCRYPT_SHA1_ALGORITHM;
			break;

		case MACAlgorithm::hmac_sha256:
			cng_algorithm = BCRYPT_SHA256_ALGORITHM;
			break;

		case MACAlgorithm::hmac_md5:
			cng_algorithm = BCRYPT_MD5_ALGORITHM;
			break;

		default:
			throw new Exception("Tls::HMAC_hash unsupported mac_algorithm", 0);
		}

		Basic::Ref<Basic::HashAlgorithm> hashAlgorithm = New<Basic::HashAlgorithm>();
		hashAlgorithm->Initialize(cng_algorithm, true);

		if (expected_output_length != hashAlgorithm->hash_output_length)
			throw new Exception("expected_output_length != hashAlgorithm->hash_output_length", 0);

		Hash(hashAlgorithm, secret, secret_length, seed, seed_count, output, expected_output_length);
	}

	void Globals::Hash(Basic::HashAlgorithm* hashAlgorithm, byte* secret, uint32 secret_length, ISerializable** seed, uint32 seed_count, byte* output, uint32 output_max)
	{
		HashStream::Ref hashStream = New<HashStream>();
		hashStream->Initialize(hashAlgorithm, secret, secret_length, output, output_max);

		for (uint32 i = 0; i < seed_count; i++)
		{
			seed[i]->SerializeTo(hashStream);
		}

		hashStream->WriteEOF();
	}

	bool Globals::SelectCipherSuite(CipherSuites* proposed_cipher_suites, CipherSuite* selected_cipher_suite)
	{
		for (CipherSuites::iterator supported_cipher_suite_it = this->supported_cipher_suites.begin(); supported_cipher_suite_it != this->supported_cipher_suites.end(); supported_cipher_suite_it++)
		{
			CipherSuites::iterator proposed_cipher_suite_it = std::find(proposed_cipher_suites->begin(), proposed_cipher_suites->end(), (*supported_cipher_suite_it));
			if (proposed_cipher_suite_it != proposed_cipher_suites->end())
			{
				(*selected_cipher_suite) = (*supported_cipher_suite_it);
				return true;
			}
		}

		return Basic::globals->HandleError("Tls::SelectCipherSuite no match", 0);
	}

	void Globals::Partition(std::vector<opaque>* source, uint16 length, std::vector<opaque>* destination)
	{
		if (length > 0)
		{
			if (length > source->size())
				throw new Exception("Tls::Partition length > source.size()", 0);

			destination->insert(destination->begin(), source->begin(), source->begin() + length);
			source->erase(source->begin(), source->begin() + length);
		}
	}
}
