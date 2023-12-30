// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.HashAlgorithm.h"

namespace Basic
{
    class HashStream : public ArrayStream<byte>
    {
    private:
        byte hash_object[0x1000] = { 0 };
        BCRYPT_HASH_HANDLE hash;
        byte* output = 0;
        uint32 output_length = 0;

    public:
        HashStream();
        virtual ~HashStream();

        void Initialize(HashAlgorithm* algorithm, byte* secret, uint32 secret_length, byte* output, uint32 output_max);

        virtual void IStream<byte>::write_elements(const byte* elements, uint32 count);
        virtual void IStream<byte>::write_eof();
    };
}