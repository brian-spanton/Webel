// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.LogFilter.h"

namespace Basic
{
    LogFilter::LogFilter(std::shared_ptr<ILog> log) :
        log(log)
    {
    }

    void LogFilter::add_entry(std::shared_ptr<LogEntry> entry)
    {
        if (!this->log)
            return;

        if (entry->level < this->min_level)
            return;

        // $$ implement advanced filters

        this->log->add_entry(entry);
    }
}