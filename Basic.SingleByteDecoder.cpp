// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.SingleByteDecoder.h"
#include "Basic.Globals.h"

namespace Basic
{
    SingleByteDecoder::SingleByteDecoder(std::shared_ptr<ISingleByteEncodingIndex> index) :
        index(index),
        destination(0)
    {
    }

    SingleByteDecoder::SingleByteDecoder(std::shared_ptr<ISingleByteEncodingIndex> index, IStream<Codepoint>* destination) :
        index(index),
        destination(destination)
    {
    }

    void SingleByteDecoder::set_destination(IStream<Codepoint>* destination)
    {
        this->destination = destination;
    }

    void SingleByteDecoder::EmitDecoderError(const char* error)
    {
        char full_error[0x100];
        int result = sprintf_s(full_error, "SingleByteDecoder::EmitDecoderError %s", error);
        if (result == -1)
            throw FatalError("Basic", "SingleByteDecoder::EmitDecoderError sprintf_s failed");

        Basic::LogDebug("Basic", full_error);

        Emit(0xFFFD);
    }

    void SingleByteDecoder::EmitDecoderError(byte b)
    {
        char error[0x100];
        int result = sprintf_s(error, "byte=0x%02X", b);
        if (result == -1)
            throw FatalError("Basic", "SingleByteDecoder::EmitDecoderError sprintf_s failed");

        EmitDecoderError(error);
    }

    void SingleByteDecoder::write_elements(const byte* elements, uint32 count)
    {
        for (const byte* element = elements; element != elements + count; element++)
        {
            write_element(*element);
        }
    }

    void SingleByteDecoder::write_element(byte b)
    {
        // http://encoding.spec.whatwg.org/#legacy-single-byte-encodings

        // From http://encoding.spec.whatwg.org/#encodings
        // A decoder algorithm takes a byte stream and emits a code point stream. The byte pointer is initially zero,
        // pointing to the first byte in the stream. It cannot be negative. It can be increased and decreased to point
        // to other bytes in the stream. The EOF byte is a conceptual byte representing the end of the stream. The byte 
        // pointer cannot point beyond the EOF byte. The EOF code point is a conceptual code point that is emitted once 
        // the byte stream is handled in its entirety. A decoder error indicates an error in the byte stream. Unless 
        // stated otherwise, emitting a decoder error must emit code point U+FFFD. A decoder must be invoked again when 
        // the word continue is used, when one or more code points are emitted of which none is the EOF code point, or 
        // when a decoder error is emitted.

        if (b >= 0 && b <= 0x7F)
        {
            Emit(b);
            return;
        }

        Codepoint codepoint = this->index->pointer_to_codepoint(b - 0x80);

        if (codepoint == 0)
        {
            EmitDecoderError(b);
            return;
        }

        Emit(codepoint);
    }

    void SingleByteDecoder::write_eof()
    {
    }

    void SingleByteDecoder::Emit(Codepoint codepoint)
    {
        this->destination->write_element(codepoint);
    }

    void SingleByteDecoderFactory::Initialize(std::shared_ptr<ISingleByteEncodingIndex> index)
    {
        this->index = index;
    }

    void SingleByteDecoderFactory::CreateDecoder(std::shared_ptr<IDecoder>* decoder)
    {
        std::shared_ptr<SingleByteDecoder> single_byte_decoder = std::make_shared<SingleByteDecoder>(this->index);

        (*decoder) = single_byte_decoder;
    }
}