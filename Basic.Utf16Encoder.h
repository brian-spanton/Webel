// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IEncoder.h"

namespace Basic
{
    class Utf16Encoder : public UnitStream<Codepoint>
    {
    private:
        std::wstring* destination = 0;

        void Emit(wchar_t c);
        void EncoderError(Codepoint codepoint);

    public:
        Utf16Encoder(std::wstring* destination);
        void IStream<Codepoint>::write_element(Codepoint element);
    };

    class AsciiEncoder : public UnitStream<Codepoint>
    {
    private:
        std::string* destination = 0;

        void Emit(char c);
        void EncoderError(Codepoint codepoint);

    public:
        AsciiEncoder(std::string* destination);
        void IStream<Codepoint>::write_element(Codepoint element);
    };
}