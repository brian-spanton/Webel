// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ICompleter.h"
#include "Basic.IStream.h"

namespace Basic
{
    class LogFile : public ICompleter, public std::enable_shared_from_this<LogFile>
    {
    private:
        static byte encoding[];

        Lock tail_lock;
        std::shared_ptr<ByteString> tail[0x400];
        uint32 bookmark;

    public:
        HANDLE file;

        LogFile();
        virtual ~LogFile();

        void Initialize(const char* name);

        void write_entry(UnicodeString* entry);
        void close_file();
        void WriteTo(IStream<Codepoint>* stream);

        virtual void ICompleter::complete(std::shared_ptr<void> context, uint32 count, uint32 error);
    };
}