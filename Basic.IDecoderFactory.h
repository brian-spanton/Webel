// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IDecoder.h"

namespace Basic
{
    __interface IDecoderFactory
    {
        void CreateDecoder(std::shared_ptr<IDecoder>* value);
    };
}