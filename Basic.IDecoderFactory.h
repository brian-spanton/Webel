// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IDecoder.h"

namespace Basic
{
    __interface IDecoderFactory : public IRefCounted
    {
        void CreateDecoder(Ref<IDecoder>* value);
    };
}