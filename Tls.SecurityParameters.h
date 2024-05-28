// Copyright © 2013 Brian Spanton

#pragma once

#include "Tls.Types.h"

namespace Tls
{
    using namespace Basic;

    class SecurityParameters
    {
    public:
        PRFAlgorithm prf_algorithm;
        BulkCipherAlgorithm bulk_cipher_algorithm;
        CipherType cipher_type;
        uint8 enc_key_length;
        uint8 block_length;
        uint8 fixed_iv_length;
        uint8 record_iv_length;
        MACAlgorithm mac_algorithm;
        uint8 mac_length;
        uint8 mac_key_length;
        CompressionMethod compression_algorithm;
        ByteString master_secret;
        Random client_random;
        Random server_random;
        uint32 verify_data_length;

        SecurityParameters();
        virtual ~SecurityParameters();

        bool InitializeCipherSuite(ProtocolVersion version, CipherSuite cipher_suite, KeyExchangeAlgorithm* key_exchange_algorithm);
    };
}