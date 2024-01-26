// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Utf8Decoder.h"
#include "Basic.Globals.h"

namespace Basic
{
    void Utf8Decoder::set_destination(IStream<Codepoint>* destination)
    {
        this->destination = destination;

        // http://encoding.spec.whatwg.org/#the-encoding
        this->codepoint = 0;
        this->seen = 0;
        this->needed = 0;
        this->lower_bound = 0x80;
        this->upper_bound = 0xBF;
    }

    void Utf8Decoder::EmitDecoderError(const char* error)
    {
        char full_error[0x100];
        int result = sprintf_s(full_error, "needed=%d seen=%d %hs", this->needed, this->seen, error);
        if (result == -1)
            throw FatalError("Basic", "Utf8Decoder", "EmitDecoderError", "sprintf_s", result);

        Basic::LogDebug("Basic", "Utf8Decoder", "EmitDecoderError", full_error);

        Emit(0xFFFD);
    }

    void Utf8Decoder::EmitDecoderError(byte b)
    {
        char error[0x100];
        sprintf_s(error, "byte=0x%02X %d", b, b);

        EmitDecoderError(error);
    }

    void Utf8Decoder::write_elements(const byte* elements, uint32 count)
    {
        for (const byte* element = elements; element != elements + count; element++)
        {
            write_element(*element);
        }
    }

    void Utf8Decoder::write_element(byte b)
    {
        // From http://encoding.spec.whatwg.org/#encodings
        // A decoder algorithm takes a byte stream and emits a code point stream. The byte pointer is initially zero,
        // pointing to the first byte in the stream. It cannot be negative. It can be increased and decreased to point
        // to other bytes in the stream. The EOF byte is a conceptual byte representing the end of the stream. The byte 
        // pointer cannot point beyond the EOF byte. The EOF code point is a conceptual code point that is emitted once 
        // the byte stream is handled in its entirety. A decoder error indicates an error in the byte stream. Unless 
        // stated otherwise, emitting a decoder error must emit code point U+FFFD. A decoder must be invoked again when 
        // the word continue is used, when one or more code points are emitted of which none is the EOF code point, or 
        // when a decoder error is emitted. 
    
        // http://encoding.spec.whatwg.org/#utf-8-decoder

        if (this->needed == 0)
        {
            if (b >= 0 && b <= 0x7F)
            {
                Emit(b);
                return;
            }
            else if (b >= 0xC2 && b <= 0xDF)
            {
                this->needed = 1;
                this->codepoint = b - 0xC0;
            }
            else if (b >= 0xE0 && b <= 0xEF)
            {
                if (b == 0xE0)
                    this->lower_bound = 0xA0;
                else if (b == 0xED)
                    this->upper_bound = 0x9F;

                this->needed = 2;
                this->codepoint = b - 0xE0;
            }
            else if (b >= 0xF0 && b <= 0xF4)
            {
                if (b == 0xF0)
                    this->lower_bound = 0x90;
                else if (b == 0xF4)
                    this->upper_bound = 0x8F;

                this->needed = 3;
                this->codepoint = b - 0xF0;
            }
            else
            {
                EmitDecoderError(b);
                return;
            }

            // (byte is in the range 0xC2 to 0xF4)
            this->codepoint <<= 6 * this->needed;
            return;
        }

        if (!(b >= this->lower_bound && b <= this->upper_bound))
        {
            this->codepoint = 0;
            this->needed = 0;
            this->seen = 0;
            this->lower_bound = 0x80;
            this->upper_bound = 0xBF;

            EmitDecoderError(b);
            write_element(b);
            return;
        }

        this->lower_bound = 0x80;
        this->upper_bound = 0xBF;
        this->seen++;
        this->codepoint += (b - 0x80) << (6 * (this->needed - this->seen));

        if (this->seen != this->needed)
            return;

        Codepoint codepoint = this->codepoint;

        this->codepoint = 0;
        this->needed = 0;
        this->seen = 0;

        Emit(codepoint);
    }

    void Utf8Decoder::write_eof()
    {
        if (this->needed != 0)
        {
            this->needed = 0;

            EmitDecoderError("end of stream with needed != 0");
        }

        // write_eof means "you will not receive any more calls to IStream interface".
        // It should not propagate to the destination, because destination might be composite
    }

    void Utf8Decoder::Emit(Codepoint codepoint)
    {
        this->destination->write_element(codepoint);
    }

    void Utf8DecoderFactory::CreateDecoder(std::shared_ptr<IDecoder>* decoder)
    {
        (*decoder) = std::make_shared<Utf8Decoder>();
    }
}