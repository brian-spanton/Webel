// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ILog.h"

namespace Basic
{
    class DebugLog : public ILog, public ArrayStream<byte>
    {
    public:
        virtual void ILog::write_entry(UnicodeStringRef entry);
        virtual void IStream<byte>::write_elements(const byte* elements, uint32 count);
    };
}