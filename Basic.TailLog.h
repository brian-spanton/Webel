// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ILog.h"
#include "Basic.IStreamWriter.h"

namespace Basic
{
    class TailLog : public ILog, public IStreamWriter<Codepoint>
    {
    private:
        std::shared_ptr<LogEntry> tail[0x100];
        uint32 bookmark;

    public:
        TailLog();

        virtual void ILog::add_entry(std::shared_ptr<LogEntry> entry);
        virtual void IStreamWriter<Codepoint>::write_to_stream(IStream<Codepoint>* stream) const;
    };
}