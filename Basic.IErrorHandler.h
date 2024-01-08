// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.TextWriter.h"

namespace Basic
{
    enum LogLevel
    {
        Debug,
        Warning,
        Critical,
    };

    __interface IErrorHandler
    {
        bool Log(LogLevel level, const char* component, const char* context, uint32 code);
        Basic::IStream<Codepoint>* LogStream();
        Basic::TextWriter* DebugWriter();
    };
}