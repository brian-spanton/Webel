// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.HashAlgorithm.h"
#include "Basic.HashStream.h"
#include "Basic.Globals.h"
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
        //this->supported_cipher_suites.push_back(CipherSuite::cs_TLS_DHE_DSS_WITH_3DES_EDE_CBC_SHA); // $$$
        this->supported_cipher_suites.push_back(CipherSuite::cs_TLS_RSA_WITH_RC4_128_MD5);

        initialize_ascii(&client_finished_label, "client finished");
        initialize_ascii(&server_finished_label, "server finished");
        initialize_ascii(&master_secret_label, "master secret");
        initialize_ascii(&key_expansion_label, "key expansion");
    }

    void Globals::PRF(PRFAlgorithm prf_algorithm, IVector<byte>* secret, ByteString* label, IStreamWriter<byte>** seed, uint32 seed_count, byte* output, uint32 output_length)
    {
        Serializer<ByteString> label_serializer(label);

        std::vector<IStreamWriter<byte>*> currentSeed;
        currentSeed.insert(currentSeed.end(), &label_serializer);
        currentSeed.insert(currentSeed.end(), seed, seed + seed_count);

        PRF_hash(prf_algorithm, secret, &*currentSeed.begin(), currentSeed.size(), output, output_length);
    }

    void Globals::PRF_hash(PRFAlgorithm prf_algorithm, IVector<byte>* secret, IStreamWriter<byte>** seed, uint32 seed_count, byte* output, uint32 output_length)
    {
        switch(prf_algorithm)
        {
        case PRFAlgorithm::tls_prf_tls_v1:
            {
                uint32 S_length = (secret->size() + 1) / 2;
                byte* S1 = secret->address();
                byte* S2 = S1 + secret->size() - S_length;

                ByteString md5_stream;
                md5_stream.resize(2 * output_length);
                P_hash(BCRYPT_MD5_ALGORITHM, S1, S_length, seed, seed_count, md5_stream.address(), output_length, md5_stream.size());

                md5_stream.resize(output_length);

                ByteString sha1_stream;
                sha1_stream.resize(2 * output_length);
                P_hash(BCRYPT_SHA1_ALGORITHM, S2, S_length, seed, seed_count, sha1_stream.address(), output_length, sha1_stream.size());

                sha1_stream.resize(output_length);

                for (uint32 i = 0; i < output_length; i++)
                    output[i] = md5_stream[i] ^ sha1_stream[i];
            }
            break;

        case PRFAlgorithm::tls_prf_sha256:
            {
                uint32 S_length = secret->size();
                byte* S1 = secret->address();

                ByteString sha256_stream;
                sha256_stream.resize(3 * output_length); // $$$ magic number = what?  2x didn't work because 2 * 12 == 24, but we got 32 empirically 
                P_hash(BCRYPT_SHA256_ALGORITHM, S1, S_length, seed, seed_count, sha256_stream.address(), output_length, sha256_stream.size());

                sha256_stream.resize(output_length);

                // $$$ is it really necessary to call P_hash with extra buffer and then copy it?  If so, optimize.
                for (uint32 i = 0; i < output_length; i++)
                    output[i] = sha256_stream[i];
            }
            break;

        default:
            throw FatalError("Tls", "Globals", "PRF_hash", "unsupported PRFAlgorithm", prf_algorithm);
        }
    }

    void Globals::P_hash(LPCWSTR algorithm, byte* secret, uint32 secret_length, IStreamWriter<byte>** seed, uint32 seed_count, byte* output, uint32 output_min, uint32 output_max)
    {
        std::shared_ptr<Basic::HashAlgorithm> hashAlgorithm = std::make_shared<Basic::HashAlgorithm>();
        hashAlgorithm->Initialize(algorithm, true);

        std::vector<IStreamWriter<byte>*> a_minus_one;
        a_minus_one.insert(a_minus_one.end(), seed, seed + seed_count);

        // make sure this is outside the loop so it doesn't get deleted before it's used
        ByteString a_current;

        for(uint32 generated_length = 0; generated_length < output_min; generated_length += hashAlgorithm->hash_output_length)
        {
            a_current.resize(hashAlgorithm->hash_output_length);

            Hash(hashAlgorithm.get(), secret, secret_length, &*a_minus_one.begin(), a_minus_one.size(), a_current.address(), a_current.size());

            Serializer<ByteString> a_current_serializer(&a_current);

            a_minus_one.clear();
            a_minus_one.push_back(&a_current_serializer);

            std::vector<IStreamWriter<byte>*> p_seed;
            p_seed.insert(p_seed.end(), &a_current_serializer);
            p_seed.insert(p_seed.end(), seed, seed + seed_count);

            Hash(hashAlgorithm.get(), secret, secret_length, &*p_seed.begin(), p_seed.size(), output + generated_length, output_max - generated_length);
        }
    }

    void Globals::HMAC_hash(MACAlgorithm mac_algorithm, ByteString* secret, IStreamWriter<byte>** seed, uint32 seed_count, byte* output, uint32 expected_output_length)
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
            throw FatalError("Tls", "Globals", "HMAC_hash", "unsupported mac_algorithm", mac_algorithm);
        }

        std::shared_ptr<Basic::HashAlgorithm> hashAlgorithm = std::make_shared<Basic::HashAlgorithm>();
        hashAlgorithm->Initialize(cng_algorithm, true);

        if (expected_output_length != hashAlgorithm->hash_output_length)
            throw FatalError("Tls", "Globals", "HMAC_hash", "expected_output_length != hashAlgorithm->hash_output_length", hashAlgorithm->hash_output_length);

        Hash(hashAlgorithm.get(), secret->address(), secret->size(), seed, seed_count, output, expected_output_length);
    }

    void Globals::Hash(Basic::HashAlgorithm* hashAlgorithm, byte* secret, uint32 secret_length, IStreamWriter<byte>** seed, uint32 seed_count, byte* output, uint32 output_max)
    {
        std::shared_ptr<HashStream> hash_stream = std::make_shared<HashStream>();
        hash_stream->Initialize(hashAlgorithm, secret, secret_length, output, output_max);

        for (uint32 i = 0; i < seed_count; i++)
        {
            seed[i]->write_to_stream(hash_stream.get());
        }

        hash_stream->write_eof();
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

        // use LogDebug instead of LogError so we are less vulnerable to a logging attack
        Basic::LogDebug("Tls", "Globals", "SelectCipherSuite", "no match");
        return false;
    }

    void Globals::Partition(ByteString* source, uint16 length, IStream<byte>* destination)
    {
        if (length > 0)
        {
            if (length > source->size())
                throw FatalError("Tls", "Globals", "Partition", "length > source.size()", length);

            destination->write_elements(source->address(), length);
            source->erase(source->begin(), source->begin() + length);
        }
    }
}
