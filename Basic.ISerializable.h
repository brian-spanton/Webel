// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IRefCounted.h"
#include "Basic.IStream.h"

namespace Basic
{
    __interface ISerializable : public IRefCounted
    {
        void SerializeTo(IStream<byte>* stream);
    };
}