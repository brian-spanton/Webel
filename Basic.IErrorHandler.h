// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.TextWriter.h"

namespace Basic
{
    __interface IErrorHandler
    {
        bool HandleError(const char* context, uint32 error);
        Basic::IStream<Codepoint>* LogStream();
        Basic::TextWriter* DebugWriter();
    };
}