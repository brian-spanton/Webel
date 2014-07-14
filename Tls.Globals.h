// Copyright © 2013 Brian Spanton

#pragma once

#include "Tls.Types.h"
#include "Tls.SecurityParameters.h"
#include "Basic.HashAlgorithm.h"

namespace Tls
{
    class Globals
    {
    public:
        CipherSuites supported_cipher_suites;

        void Initialize();

        void PRF(PRFAlgorithm prf_algorithm, IVector<byte>* secret, ByteString* label, IStreamWriter<byte>** seed, uint32 seed_count, byte* output, uint32 output_length);
        void PRF_hash(PRFAlgorithm prf_algorithm, IVector<byte>* secret, IStreamWriter<byte>** seed, uint32 seed_count, byte* output, uint32 output_length);
        void P_hash(LPCWSTR algorithm, byte* secret, uint32 secret_length, IStreamWriter<byte>** seed, uint32 seed_count, byte* output, uint32 output_min, uint32 output_max);
        void HMAC_hash(MACAlgorithm mac_algorithm, ByteString* secret, IStreamWriter<byte>** seed, uint32 seed_count, byte* output, uint32 expected_output_length);
        void Hash(Basic::HashAlgorithm* hashAlgorithm, byte* secret, uint32 secret_length, IStreamWriter<byte>** seed, uint32 seed_count, byte* output, uint32 output_max);
        bool SelectCipherSuite(CipherSuites* proposed_cipher_suites, CipherSuite* selected_cipher_suite);
        void Partition(ByteString* source, uint16 length, IStream<byte>* destination);

        ByteString client_finished_label;
        ByteString server_finished_label;
        ByteString master_secret_label;
        ByteString key_expansion_label;

        static const int master_secret_length = 48;
    };

    extern Globals* globals;
}