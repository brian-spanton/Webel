// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.LogFile.h"
#include "Basic.Globals.h"

namespace Basic
{
    byte LogFile::encoding[] = { 0xef, 0xbb, 0xbf };

    LogFile::LogFile() :
        file(INVALID_HANDLE_VALUE),
        bookmark(0)
    {
    }

    LogFile::~LogFile()
    {
        close_file();
    }

    void LogFile::Initialize(const char* name)
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

        Basic::globals->BindToCompletionQueue(this);

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

    void LogFile::write_entry(UnicodeString* entry)
    {
        std::shared_ptr<ByteString> bytes;

        {
            Hold hold(tail_lock);

            // first thing get the ByteString to reuse from the circular array
            bytes = this->tail[this->bookmark];
            if (bytes.get() == 0)
            {
                // first time we've used this slot, so allocate the ByteString
                bytes = std::make_shared<ByteString>();
                this->tail[this->bookmark] = bytes;
            }

            // advance the circular array bookmark
            this->bookmark = (this->bookmark + 1) % _countof(this->tail);
        }

        // ascii encode and write to the local consoles
        ascii_encode(entry, bytes.get());
        OutputDebugStringA((char*)bytes->address());
        printf((char*)bytes->address());

        if (this->file != INVALID_HANDLE_VALUE)
        {
            // the log file is utf-8 encoded, pre-allocate space so the encoding 
            // doesn't make a lot of incrementally increasing allocations
            bytes->clear();
            bytes->reserve(entry->size() * 5 / 4);
            utf_8_encode(entry, bytes.get());

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
    }

    void LogFile::close_file()
    {
        FlushFileBuffers(this->file);
        CloseHandle(this->file);
        this->file = INVALID_HANDLE_VALUE;
    }

    void LogFile::complete(std::shared_ptr<void> context, uint32 count, uint32 error)
    {
        if (error != ERROR_SUCCESS)
        {
            close_file();

            Basic::globals->HandleError("LogFile::CompleteAsync", error);
        }
    }

    void LogFile::WriteTo(IStream<Codepoint>* stream)
    {
        Hold hold(this->tail_lock);

        for (uint32 i = 0; i < _countof(this->tail); i++)
        {
            uint32 next = (this->bookmark + i) % _countof(this->tail);

            UnicodeString entry;

            ByteString* bytes = this->tail[next].get();
            utf_8_decode(bytes, &entry);

            entry.write_to_stream(stream);
        }
    }
}