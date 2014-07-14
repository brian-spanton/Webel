// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Cng.h"

namespace Basic
{
    class HashAlgorithm
    {
    public:
        Basic::BCRYPT_ALG_HANDLE hash_algorithm;
        DWORD hash_output_length;
        bool hmac;

        HashAlgorithm();
        virtual ~HashAlgorithm();

        void Initialize(LPCWSTR algorithm, bool hmac);
    };
}