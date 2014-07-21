// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ILog.h"

namespace Basic
{
    class MemoryLog : public ILog
    {
    private:
        typedef std::list<UnicodeStringRef> EntryList;

        Lock lock;
        EntryList entries;

    public:
        MemoryLog();

        virtual void ILog::write_entry(UnicodeStringRef entry);
        void WriteTo(IStream<Codepoint>* stream);
        void WriteTo(ILog* log);
    };
}