// Copyright © 2013 Brian Spanton

#pragma once

#include "Tls.Types.h"
#include "Tls.SecurityParameters.h"
#include "Basic.Cng.h"

namespace Tls
{
    class ConnectionState
    {
    public:
        // compression_state
        Basic::BCRYPT_KEY_HANDLE key_handle;

        ByteString MAC_key;
        ByteString encryption_key;
        std::shared_ptr<ByteString> IV;

        uint64 sequence_number;
        std::shared_ptr<SecurityParameters> security_parameters;

        ConnectionState();
        virtual ~ConnectionState();

        bool Initialize(std::shared_ptr<SecurityParameters> params);
        void MAC(Record* compressed, byte* output, uint8 output_max);
    };

}