// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IEncoder.h"

namespace Basic
{
    __interface IEncoderFactory
    {
        void CreateEncoder(std::shared_ptr<IEncoder>* value);
    };
}