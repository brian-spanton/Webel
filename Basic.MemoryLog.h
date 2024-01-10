// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ILog.h"

namespace Basic
{
    class MemoryLog : public ILog
    {
    private:
        typedef std::list<std::shared_ptr<LogEntry> > EntryList;

        EntryList entries;

    public:
        MemoryLog();

        virtual void ILog::add_entry(std::shared_ptr<LogEntry> entry);
    };
}