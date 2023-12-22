// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.SingleByteDecoder.h"

namespace Basic
{
    class TextWriter
    {
    private:
        IStream<Codepoint>* dest = 0;

    public:
        SingleByteDecoder decoder;

        TextWriter();
        TextWriter(IStream<Codepoint>* dest);

        void Initialize(IStream<Codepoint>* dest);

        void write_elements(const char* text, uint32 count);
        void write_c_str(const char* text);
        void WriteLine(const char* text);
        void WriteLine();
        void WriteThreadId();
        void WriteTimestamp();
        void WriteError(uint32 error);

        template <int count>
        void write_literal(const char (&text)[count])
        {
            if (text[count - 1] == 0)
                return write_elements(text, count - 1);
            else
                return write_elements(text, count);
        }

        template<int max>
        void WriteFormat(const char* format, va_list args)
        {
            char temp[max + 1];
            uint32 count = vsprintf_s(temp, format, args);
            if (count < 0)
                throw FatalError("WriteFormat failed");

            return write_elements(temp, count);
        }

        template<int max>
        void WriteFormat(const char* format, ...)
        {
            va_list args;
            va_start(args, format);
            WriteFormat<max>(format, args);
            va_end(args);
        }
    };
}