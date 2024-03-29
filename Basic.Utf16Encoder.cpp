// Copyright � 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Utf16Encoder.h"
#include "Basic.Globals.h"

namespace Basic
{
    Utf16Encoder::Utf16Encoder(std::wstring* destination) :
        destination(destination)
    {
    }

    void Utf16Encoder::EncoderError(Codepoint codepoint)
    {
        char message[0x100];
        int result = sprintf_s(message, "codepoint=0x%04X", codepoint);
        if (result == -1)
            throw FatalError("Basic", "Utf16Encoder", "EncoderError", "sprintf_s error", result);

        Basic::LogDebug("Basic", "Utf16Encoder", "EncoderError", message);
    }

    void Utf16Encoder::write_element(Codepoint codepoint)
    {
        // https://en.wikipedia.org/wiki/UTF-16

        if (codepoint == EOF)
        {
            Basic::LogDebug("Basic", "Utf16Encoder", "write_element", "codepoint == EOF (unexpected eof)");
            return;
        }

        if (codepoint >= 0 && codepoint <= 0xFFFF)
        {
            Emit(static_cast<wchar_t>(codepoint));
            return;
        }

        if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
        {
            EncoderError(codepoint);
            return;
        }

        uint32 offset = codepoint - 0x10000;

        Emit(static_cast<wchar_t>((offset >> 10) + 0xD800)); // lead/high surrogate
        Emit(static_cast<wchar_t>((offset & 0x3FF) + 0xDC00)); // low surrogate
    }

    void Utf16Encoder::Emit(wchar_t c)
    {
        this->destination->push_back(c);
    }

    AsciiEncoder::AsciiEncoder(std::string* destination) :
        destination(destination)
    {
    }

    void AsciiEncoder::EncoderError(Codepoint codepoint)
    {
        char message[0x100];
        int result = sprintf_s(message, "codepoint=0x%04X", codepoint);
        if (result == -1)
            throw FatalError("Basic", "Utf16Encoder", "EncodeError", "sprintf_s error", result);

        Basic::LogDebug("Basic", "Utf16Encoder", "EncodeError", message);
    }

    void AsciiEncoder::write_element(Codepoint codepoint)
    {
        if (codepoint == EOF)
        {
            Basic::LogDebug("Basic", "Utf16Encoder", "write_element", "codepoint == EOF (unexpected eof)");
            return;
        }

        if (codepoint >= 0 && codepoint <= 0x7F)
        {
            Emit((char)codepoint);
            return;
        }

        EncoderError(codepoint);
        Emit(0x7F);
    }

    void AsciiEncoder::Emit(char c)
    {
        this->destination->push_back(c);
    }
}