// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IJobEventHandler.h"
#include "Basic.ILog.h"

namespace Basic
{
    class FileLog : public IJobEventHandler, public std::enable_shared_from_this<FileLog>, public ILog
    {
    private:
        static byte encoding[];

    public:
        HANDLE file;

        FileLog();
        virtual ~FileLog();

        void Initialize(const char* name);

        virtual void ILog::write_entry(UnicodeStringRef entry);
        void close_file();

        virtual void IJobEventHandler::job_completed(std::shared_ptr<void> context, uint32 count, uint32 error);
    };
}