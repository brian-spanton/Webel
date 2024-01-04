// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ICompleter.h"
#include "Basic.ILog.h"

namespace Basic
{
    class FileStream : public ICompleter, public std::enable_shared_from_this<FileStream>, public ArrayStream<byte>
    {
    public:
        HANDLE file;
        DWORD position = 0;

        FileStream();
        virtual ~FileStream();

        void Initialize(const char* name);

        virtual void IStream<byte>::write_elements(const byte* elements, uint32 count);
        virtual void IStream<byte>::write_eof();
        void close_file();

        virtual void ICompleter::complete(std::shared_ptr<void> context, uint32 count, uint32 error);
    };
}