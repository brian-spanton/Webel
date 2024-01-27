// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ILog.h"

namespace Basic
{
    class LogFilter : public ILog
    {
    private:
        std::shared_ptr<ILog> log;

    public:
        LogLevel min_level = LogLevel::Debug;

        LogFilter(std::shared_ptr<ILog> log);

        virtual void ILog::add_entry(std::shared_ptr<LogEntry> entry);
    };
}