// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ILog.h"

namespace Basic
{
    class DebugLog : public ILog
    {
    public:
        virtual void ILog::write_entry(UnicodeStringRef entry);
    };
}