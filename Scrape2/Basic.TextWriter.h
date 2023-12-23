// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.SingleByteDecoder.h"

namespace Basic
{
    class TextWriter
    {
    private:
        IStream<Codepoint>* dest;

    public:
        SingleByteDecoder decoder;

        TextWriter();
        TextWriter(IStream<Codepoint>* dest);

        void Initialize(IStream<Codepoint>* dest);

        void write_elements(const char* text, uint32 count);
        void write_c_str(const char* text);
        void write_line(const char* text);
        void write_line();
        void write_thread_id();
        void write_timestamp();
        void write_error_code(uint32 error);
        void HandleError(const char* context, uint32 error);

        template <int count>
        void write_literal(const char (&text)[count])
        {
            if (text[count - 1] == 0)
                return write_elements(text, count - 1);
            else
                return write_elements(text, count);
        }

        template<int max>
        void write_format(const char* format, va_list args)
        {
            char temp[max + 1];
            uint32 count = vsprintf_s(temp, format, args);
            if (count < 0)
                throw FatalError("write_format failed");

            return write_elements(temp, count);
        }

        template<int max>
        void write_format(const char* format, ...)
        {
            va_list args;
            va_start(args, format);
            write_format<max>(format, args);
            va_end(args);
        }
    };
}