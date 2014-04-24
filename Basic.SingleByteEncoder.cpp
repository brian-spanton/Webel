// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.SingleByteEncoder.h"
#include "Basic.Globals.h"
#include "Basic.SingleByteEncodingIndex.h"

namespace Basic
{
    SingleByteEncoder::SingleByteEncoder()
        : error_replacement_byte(0x7F) // $ is this a good default error_replacement_byte char?
    {
    }

    void SingleByteEncoder::Initialize(ISingleByteEncodingIndex* index)
    {
        this->index = index;
    }

    void SingleByteEncoder::Initialize(ISingleByteEncodingIndex* index, IStream<byte>* destination)
    {
        this->index = index;
        this->destination = destination;
    }

    void SingleByteEncoder::set_destination(IStream<byte>* destination)
    {
        this->destination = destination;
    }

    void SingleByteEncoder::set_error_replacement_byte(byte error_replacement_byte)
    {
        this->error_replacement_byte = error_replacement_byte;
    }

    void SingleByteEncoder::EncoderError(Codepoint codepoint)
    {
        char error[0x100];
        int result = sprintf_s(error, "codepoint=0x%04X", codepoint);
        if (result == -1)
            throw new Exception("sprintf_s");

        HandleError(error);
    }

    void SingleByteEncoder::Write(const Codepoint* elements, uint32 count)
    {
        // From http://encoding.spec.whatwg.org/#encodings
        // An encoder algorithm takes a code point stream and emits a byte stream. It fails when a code point is passed for
        // which it does not have a corresponding byte (sequence). Analogously to a encoder, it has a code point pointer and
        // encoder error. An encoder must be invoked again when the word continue is used, or when one or more bytes are emitted 
        // of which none is the EOF byte. Unless stated otherwise, when an encoder error is emitted the encoder terminates. 
        //
        // Note: HTML forms and URLs require non-terminating encoders and have therefore special handling whenever an encoder error
        // is reached. Instead of terminating the encoder one or more code points in the range U+0000 to U+007F are inserted into
        // the code point stream at code point pointer after encoder error is emitted. 
    
        // http://encoding.spec.whatwg.org/#legacy-single-byte-encodings

        uint32 i = 0;

        while (true)
        {
            if (i == count)
                break;

            Codepoint codepoint = elements[i];

            i++;

            if (codepoint >= 0 && codepoint <= 0x7F)
            {
                Emit((byte)codepoint);
                continue;
            }

            byte pointer = this->index->codepoint_to_pointer(codepoint);

            if (pointer == 0)
            {
                EncoderError(codepoint);
                Emit(this->error_replacement_byte);
                continue;
            }

            Emit(pointer + 0x80);
            continue;
        }
    }

    void SingleByteEncoder::WriteEOF()
    {
    }

    void SingleByteEncoder::Emit(byte b)
    {
        this->destination->Write(&b, 1);
    }

    void SingleByteEncoderFactory::Initialize(ISingleByteEncodingIndex* index)
    {
        this->index = index;
    }

    void SingleByteEncoderFactory::CreateEncoder(Basic::Ref<IEncoder>* encoder)
    {
        SingleByteEncoder::Ref single_byte_encoder = New<SingleByteEncoder>();
        single_byte_encoder->Initialize(this->index);

        (*encoder) = single_byte_encoder;
    }
}