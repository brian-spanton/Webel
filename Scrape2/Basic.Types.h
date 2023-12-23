// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IDecoderFactory.h"
#include "Basic.IEncoderFactory.h"

namespace Basic
{
    struct FatalError
    {
        FatalError(const char* context);
        FatalError(const char* context, uint32 error);
    };

    typedef StringMapCaseInsensitive<std::shared_ptr<IEncoderFactory> > EncoderMap;
    typedef StringMapCaseInsensitive<std::shared_ptr<IDecoderFactory> > DecoderMap;
}
