// Copyright � 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.FileLog.h"
#include "Basic.Globals.h"

namespace Basic
{
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
            throw FatalError("Basic", "FileLog", "Initialize", "::CreateFileA", GetLastError());

        Basic::globals->BindToCompletionQueue(this->file);

        LARGE_INTEGER size;
        BOOL success = GetFileSizeEx(this->file, &size);
        if (success == FALSE)
            throw FatalError("Basic", "FileLog", "Initialize", "GetFileSizeEx", GetLastError());

        if (size.QuadPart == 0)
        {
            std::shared_ptr<ByteString> bytes = std::make_shared<ByteString>();
            bytes->write_elements(Globals::utf_8_bom, sizeof(Globals::utf_8_bom));

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

    void FileLog::add_entry(std::shared_ptr<LogEntry> entry)
    {
        std::shared_ptr<ByteString> bytes = std::make_shared<ByteString>();
        entry->render_utf8(bytes.get());
        bytes->append(reinterpret_cast<byte*>("\r\n"), 2);

        std::shared_ptr<Job> job = Job::make(this->shared_from_this(), bytes);
        job->Offset = this->position;
        job->OffsetHigh = 0;
        this->position += bytes->size();

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

            Basic::LogError("Basic", "FileLog", "complete", "error != ERROR_SUCCESS", error);
        }
    }
}