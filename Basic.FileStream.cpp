// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.FileStream.h"
#include "Basic.Globals.h"

namespace Basic
{
    FileStream::FileStream() :
        file(INVALID_HANDLE_VALUE)
    {
    }

    FileStream::~FileStream()
    {
        close_file();
    }

    void FileStream::Initialize(const char* name)
    {
        this->file = ::CreateFileA(
            name,
            GENERIC_WRITE,
            FILE_SHARE_READ,
            0,
            CREATE_ALWAYS,
            FILE_FLAG_OVERLAPPED,
            0);
        if (this->file == INVALID_HANDLE_VALUE)
            throw FatalError("Basic", "FileStream::Initialize { CreateFileA }", GetLastError());

        this->position = 0;

        Basic::globals->BindToCompletionQueue(this->file);
    }

    void FileStream::write_elements(const byte* elements, uint32 count)
    {
        std::shared_ptr<ByteString> bytes = std::make_shared<ByteString>();

        bytes->insert(bytes->end(), elements, elements + count);

        std::shared_ptr<Job> job = Job::make(this->shared_from_this(), bytes);
        job->Offset = this->position;
        job->OffsetHigh = 0;
        this->position += count;

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

    void FileStream::write_eof()
    {
        this->close_file();
    }

    void FileStream::close_file()
    {
        FlushFileBuffers(this->file);
        CloseHandle(this->file);
        this->file = INVALID_HANDLE_VALUE;
    }

    void FileStream::complete(std::shared_ptr<void> context, uint32 count, uint32 error)
    {
        if (error != ERROR_SUCCESS)
        {
            close_file();

            Basic::LogError("Basic", "FileStream::complete", error);
        }
    }
}