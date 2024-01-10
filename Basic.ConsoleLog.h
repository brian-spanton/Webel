// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ILog.h"

namespace Basic
{
    class ConsoleLog : public ILog
    {
    public:
        virtual void ILog::add_entry(std::shared_ptr<LogEntry> entry);
    };
}