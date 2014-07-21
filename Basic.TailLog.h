// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ILog.h"
#include "Basic.IStreamWriter.h"

namespace Basic
{
    class TailLog : public ILog, public IStreamWriter<Codepoint>
    {
    private:
        Lock lock;
        std::shared_ptr<UnicodeString> tail[0x100];
        uint32 bookmark;

    public:
        TailLog();

        virtual void ILog::write_entry(UnicodeStringRef entry);
        virtual void IStreamWriter<Codepoint>::write_to_stream(IStream<Codepoint>* stream) const;
        void WriteTo(ILog* log);
    };
}