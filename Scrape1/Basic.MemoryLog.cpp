// Copyright � 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.MemoryLog.h"
#include "Basic.Globals.h"

namespace Basic
{
    MemoryLog::MemoryLog()
    {
    }

    void MemoryLog::write_entry(UnicodeStringRef entry)
    {
        this->entries.push_back(entry);
    }

    void MemoryLog::WriteTo(IStream<Codepoint>* stream)
    {
        for (EntryList::iterator it = this->entries.begin(); it != this->entries.end(); it++)
        {
            (*it)->write_to_stream(stream);
        }
    }

    void MemoryLog::WriteTo(ILog* log)
    {
        for (EntryList::iterator it = this->entries.begin(); it != this->entries.end(); it++)
        {
            log->write_entry(*it);
        }
    }
}