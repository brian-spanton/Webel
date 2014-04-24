// Copyright © 2013 Brian Spanton

#pragma once

#include "Tls.Types.h"
#include "Tls.VectorFrame.h"
#include "Tls.SecurityParameters.h"
#include "Tls.CertificatesFrame.h"
#include "Basic.HashAlgorithm.h"

namespace Tls
{
    class Globals
    {
    public:
        CipherSuites supported_cipher_suites;

        void Initialize();

        void PRF(PRFAlgorithm prf_algorithm, opaque* secret, uint32 secret_length, opaque* label, uint32 label_length, ISerializable** seed, uint32 seed_count, opaque* output, uint32 output_length);
        void PRF_hash(PRFAlgorithm prf_algorithm, opaque* secret, uint32 secret_length, ISerializable** seed, uint32 seed_count, opaque* output, uint32 output_length);
        void P_hash(LPCWSTR algorithm, opaque* secret, uint32 secret_length, ISerializable** seed, uint32 seed_count, opaque* output, uint32 output_min, uint32 output_max);
        void HMAC_hash(MACAlgorithm mac_algorithm, opaque* secret, uint32 secret_length, ISerializable** seed, uint32 seed_count, opaque* output, uint32 expected_output_length);
        void Hash(Basic::HashAlgorithm* hashAlgorithm, byte* secret, uint32 secret_length, ISerializable** seed, uint32 seed_count, byte* output, uint32 output_max);
        bool SelectCipherSuite(CipherSuites* proposed_cipher_suites, CipherSuite* selected_cipher_suite);
        void Partition(std::vector<opaque>* source, uint16 length, std::vector<opaque>* destination);
    };

    extern Globals* globals;
}