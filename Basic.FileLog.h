// Copyright � 2013 Brian Spanton

#pragma once

#include "Basic.ICompleter.h"
#include "Basic.ILog.h"

namespace Basic
{
    class FileLog : public ICompleter, public std::enable_shared_from_this<FileLog>, public ILog
    {
    public:
        HANDLE file;
        DWORD position = 0;

        FileLog();
        virtual ~FileLog();

        void Initialize(const char* name);

        virtual void ILog::add_entry(std::shared_ptr<LogEntry> entry);
        void close_file();

        virtual void ICompleter::complete(std::shared_ptr<void> context, uint32 count, uint32 error);
    };
}