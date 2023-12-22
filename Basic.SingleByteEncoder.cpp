// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.SingleByteEncoder.h"
#include "Basic.Globals.h"
#include "Basic.SingleByteEncodingIndex.h"

namespace Basic
{
    SingleByteEncoder::SingleByteEncoder()
    {
    }

    void SingleByteEncoder::Initialize(std::shared_ptr<ISingleByteEncodingIndex> index)
    {
        this->index = index;
    }

    void SingleByteEncoder::Initialize(std::shared_ptr<ISingleByteEncodingIndex> index, IStream<byte>* destination)
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
            throw FatalError("sprintf_s");

        HandleError(error);
    }

    void SingleByteEncoder::write_elements(const Codepoint* elements, uint32 count)
    {
        for (const Codepoint* element = elements; element != elements + count; element++)
        {
            write_element(*element);
        }
    }

    void SingleByteEncoder::write_element(Codepoint codepoint)
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

        if (codepoint >= 0 && codepoint <= 0x7F)
        {
            Emit((byte)codepoint);
            return;
        }

        byte pointer = this->index->codepoint_to_pointer(codepoint);

        if (pointer == 0)
        {
            EncoderError(codepoint);
            Emit(this->error_replacement_byte);
            return;
        }

        Emit(pointer + 0x80);
    }

    void SingleByteEncoder::write_eof()
    {
        // I think eof is for the end of the bytes to encode, and should not propagate to the destination
        // because it might not at all be the last thing sent to the destination
    }

    void SingleByteEncoder::Emit(byte b)
    {
        this->destination->write_element(b);
    }

    void SingleByteEncoderFactory::Initialize(std::shared_ptr<ISingleByteEncodingIndex> index)
    {
        this->index = index;
    }

    void SingleByteEncoderFactory::CreateEncoder(std::shared_ptr<IEncoder>* encoder)
    {
        std::shared_ptr<SingleByteEncoder> single_byte_encoder = std::make_shared<SingleByteEncoder>();
        single_byte_encoder->Initialize(this->index);

        (*encoder) = single_byte_encoder;
    }
}