// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Utf8Encoder.h"
#include "Basic.Globals.h"

namespace Basic
{
    Utf8Encoder::Utf8Encoder() :
        error_replacement_byte(0x7F) // $ is this a good default error_replacement_byte char?
    {
    }

    void Utf8Encoder::set_destination(IStream<byte>* destination)
    {
        this->destination = destination;
    }

    void Utf8Encoder::set_error_replacement_byte(byte error_replacement_byte)
    {
        this->error_replacement_byte = error_replacement_byte;
    }

    void Utf8Encoder::EncoderError(Codepoint codepoint)
    {
        char error[0x100];
        int result = sprintf_s(error, "Utf8Encoder::EncoderError codepoint=0x%04X", codepoint);
        if (result == -1)
            throw FatalError("sprintf_s");

        HandleError(error);
    }

    void Utf8Encoder::write_elements(const Codepoint* elements, uint32 count)
    {
        for (const Codepoint* element = elements; element != elements + count; element++)
        {
            write_element(*element);
        }
    }

    void Utf8Encoder::write_element(Codepoint codepoint)
    {
        // From http://encoding.spec.whatwg.org/#encodings
        // An encoder algorithm takes a code point stream and emits a byte stream. It fails when a code point is passed for
        // which it does not have a corresponding byte (sequence). Analogously to a decoder, it has a code point pointer and
        // encoder error. An encoder must be invoked again when the word continue is used, or when one or more bytes are emitted
        // of which none is the EOF byte. Unless stated otherwise, when an encoder error is emitted the encoder terminates. 
        //
        //     Note: HTML forms and URLs require non-terminating encoders and have therefore special handling whenever an 
        //     encoder error is reached. Instead of terminating the encoder one or more code points in the range U+0000 to U+007F
        //     are inserted into the code point stream at code point pointer after encoder error is emitted. 
    
        // http://encoding.spec.whatwg.org/#utf-8-encoder

        if (codepoint == EOF)
        {
            HandleError("unexpected eof");
            return;
        }

        if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
        {
            EncoderError(codepoint);
            Emit(this->error_replacement_byte);
            return;
        }

        if (codepoint >= 0x0000 && codepoint <= 0x007F)
        {
            Emit((byte)codepoint);
            return;
        }

        byte count;
        byte offset;

        if (codepoint >= 0x0080 && codepoint <= 0x07FF)
        {
            count = 1;
            offset = 0xC0;
        }
        else if (codepoint >= 0x0800 && codepoint <= 0xFFFF)
        {
            count = 2;
            offset = 0xE0;
        }
        else if (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
        {
            count = 3;
            offset = 0xF0;
        }

        byte b = (byte)((codepoint >> (6 * count)) + offset);
        Emit(b);

        while (count > 0)
        {
            Codepoint temp = codepoint >> (6 * (count - 1));
            b = (byte)(0x80 + (temp % 64));
            Emit(b);

            count --;
        }
    }

    void Utf8Encoder::write_eof()
    {
        // I think eof is for the end of the bytes to encode, and should not propagate to the destination
        // because it might not at all be the last thing sent to the destination

        HandleError("unexpected eof");
    }

    void Utf8Encoder::Emit(byte b)
    {
        this->destination->write_element(b);
    }

    void Utf8EncoderFactory::CreateEncoder(std::shared_ptr<IEncoder>* encoder)
    {
        (*encoder) = std::make_shared<Utf8Encoder>();
    }
}