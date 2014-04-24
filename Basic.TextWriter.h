// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.SingleByteDecoder.h"

namespace Basic
{
    class TextWriter
    {
    private:
        Ref<IStream<Codepoint> > dest; // REF
        Inline<SingleByteDecoder> decoder;

    public:
        TextWriter();
        TextWriter(IStream<Codepoint>* dest);

        void Initialize(IStream<Codepoint>* dest);

        void Write(const char* text, uint32 count);
        void Write(const char* text);
        void WriteLine(const char* text);
        void WriteLine();
        void WriteThreadId();
        void WriteTimestamp();
        void WriteError(uint32 error);

        template <int count>
        void Write(char (&text)[count])
        {
            if (text[count - 1] == 0)
                return Write(text, count - 1);
            else
                return Write(text, count);
        }

        template<int max>
        void WriteFormat(const char* format, va_list args)
        {
            char temp[max + 1];
            uint32 count = vsprintf_s(temp, format, args);
            if (count < 0)
                throw new Exception("WriteFormat failed");

            return Write(temp, count);
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