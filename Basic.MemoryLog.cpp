// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.MemoryLog.h"
#include "Basic.Globals.h"

namespace Basic
{
    MemoryLog::MemoryLog()
    {
    }

    void MemoryLog::add_entry(std::shared_ptr<LogEntry> entry)
    {
        this->entries.push_back(entry);
    }
}