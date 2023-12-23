// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.HashStream.h"
#include "Basic.Globals.h"

namespace Basic
{
    HashStream::HashStream() : 
        hash(0)
    {
    }

    HashStream::~HashStream()
    {
        if (hash != 0)
        {
            NTSTATUS error = BCryptDestroyHash(hash);
            if (error != 0)
                Basic::globals->HandleError("BCryptDestroyHash", error);
        }
    }

    void HashStream::Initialize(HashAlgorithm* algorithm, byte* secret, uint32 secret_length, byte* output, uint32 output_max)
    {
        if (output_max < algorithm->hash_output_length)
            throw FatalError("HashStream::Initialize output_max < algorithm->hash_output_length", 0);

        bool hmac = secret_length > 0;
        if (hmac != algorithm->hmac)
            throw FatalError("HashStream::Initialize hmac != algorithm->hmac", 0);

        this->output = output;
        this->output_length = algorithm->hash_output_length;

        NTSTATUS error = BCryptCreateHash(algorithm->hash_algorithm, &this->hash, this->hash_object, sizeof(this->hash_object), secret, secret_length, 0);
        if (error != 0)
            throw FatalError("BCryptCreateHash", error);
    }

    void HashStream::write_elements(const byte* elements, uint32 count)
    {
        NTSTATUS error = BCryptHashData(hash, (PUCHAR)elements, count, 0);
        if (error != 0)
            throw FatalError("BCryptHashData", error);
    }

    void HashStream::write_eof()
    {
        NTSTATUS error = BCryptFinishHash(hash, this->output, this->output_length, 0);
        if (error != 0)
            throw FatalError("BCryptFinishHash", error);
    }
}