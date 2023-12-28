// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.FileLog.h"
#include "Basic.Globals.h"

namespace Basic
{
    byte FileLog::encoding[] = { 0xef, 0xbb, 0xbf };

    FileLog::FileLog() :
        file(INVALID_HANDLE_VALUE)
    {
    }

    FileLog::~FileLog()
    {
        close_file();
    }

    void FileLog::Initialize(const char* name)
    {
        this->file = ::CreateFileA(
            name,
            GENERIC_WRITE,
            FILE_SHARE_READ,
            0,
            OPEN_ALWAYS,
            FILE_FLAG_OVERLAPPED | FILE_FLAG_WRITE_THROUGH,
            0);
        if (this->file == INVALID_HANDLE_VALUE)
            throw FatalError("CreateFileA", GetLastError());

        Basic::globals->BindToCompletionQueue(this->file);

        LARGE_INTEGER size;
        BOOL success = GetFileSizeEx(this->file, &size);
        if (success == FALSE)
            throw FatalError("GetFileSizeEx", GetLastError());

        if (size.QuadPart == 0)
        {
            std::shared_ptr<ByteString> bytes = std::make_shared<ByteString>();
            bytes->write_elements(encoding, sizeof(encoding));

            std::shared_ptr<Job> job = Job::make(this->shared_from_this(), bytes);

            success = WriteFile(this->file, bytes->address(), bytes->size(), 0, job.get());
            if (success == FALSE)
            {
                DWORD error = GetLastError();
                if (error != ERROR_IO_PENDING)
                {
                    job->Internal = error;
                    Basic::globals->QueueJob(job);
                }
            }
        }
    }

    void FileLog::write_entry(UnicodeStringRef entry)
    {
        std::shared_ptr<ByteString> bytes = std::make_shared<ByteString>();

        // the log file is utf-8 encoded; pre-allocate space so the encoding 
        // doesn't make a lot of incrementally increasing allocations
        bytes->reserve(entry->size() * 5 / 4);
        utf_8_encode(entry.get(), bytes.get());

        std::shared_ptr<Job> job = Job::make(this->shared_from_this(), bytes);
        job->Offset = 0xffffffff;
        job->OffsetHigh = 0xffffffff;

        BOOL success = WriteFile(this->file, bytes->address(), bytes->size(), 0, job.get());
        if (success == FALSE)
        {
            DWORD error = GetLastError();
            if (error != ERROR_IO_PENDING)
            {
                job->Internal = error;
                Basic::globals->QueueJob(job);
            }
        }
    }

    void FileLog::close_file()
    {
        FlushFileBuffers(this->file);
        CloseHandle(this->file);
        this->file = INVALID_HANDLE_VALUE;
    }

    void FileLog::complete(std::shared_ptr<void> context, uint32 count, uint32 error)
    {
        if (error != ERROR_SUCCESS)
        {
            close_file();

            Basic::globals->HandleError("FileLog::CompleteAsync", error);
        }
    }
}