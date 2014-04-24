// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.LogFile.h"
#include "Basic.Globals.h"

namespace Basic
{
    byte LogFile::encoding[] = { 0xef, 0xbb, 0xbf };

    LogFile::LogFile() :
        file(INVALID_HANDLE_VALUE)
    {
    }

    LogFile::~LogFile()
    {
        if (this->file != INVALID_HANDLE_VALUE)
            FlushFileBuffers(this->file);
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
            throw new Exception("CreateFileA", GetLastError());

        Basic::globals->BindToCompletionQueue(this);

        LARGE_INTEGER size;
        BOOL success = GetFileSizeEx(this->file, &size);
        if (success == FALSE)
            throw new Exception("GetFileSizeEx", GetLastError());

        if (size.QuadPart == 0)
        {
            AsyncBytes::Ref bytes = New<AsyncBytes>("6");
            bytes->Initialize(0x10);
            bytes->Write(encoding, sizeof(encoding));
            bytes->PrepareForSend("LogFile::Initialize WriteFile", this);

            success = WriteFile(this->file, bytes->bytes, bytes->count, 0, bytes);
            if (success == FALSE)
            {
                DWORD error = GetLastError();
                if (error != ERROR_IO_PENDING)
                    throw new Exception("LogFile::Initialize WriteFile", error);
            }
        }
    }

    void LogFile::Write(AsyncBytes* bytes)
    {
        if (this->file == INVALID_HANDLE_VALUE)
            return;

        bytes->Offset = 0xffffffff;
        bytes->OffsetHigh = 0xffffffff;
        bytes->PrepareForSend("LogFile::Write WriteFile", this);

        BOOL success = WriteFile(this->file, bytes->bytes, bytes->count, 0, bytes);
        if (success == FALSE)
        {
            DWORD error = GetLastError();
            if (error != ERROR_IO_PENDING)
            {
                bytes->Internal = error;
                Basic::globals->PostCompletion(this, bytes);
            }
        }
    }

    void LogFile::CompleteAsync(OVERLAPPED_ENTRY& entry)
    {
        AsyncBytes* bytesPointer = AsyncBytes::FromOverlapped(entry.lpOverlapped);
        AsyncBytes::Ref bytes = bytesPointer;
        bytesPointer->IoCompleted();

        int error = static_cast<int>(entry.lpOverlapped->Internal);
        if (error != ERROR_SUCCESS)
            Basic::globals->HandleError("LogFile::CompleteAsync", error);
    }
}