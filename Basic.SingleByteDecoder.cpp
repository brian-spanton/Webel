// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.SingleByteDecoder.h"
#include "Basic.Globals.h"

namespace Basic
{
    void SingleByteDecoder::Initialize(ISingleByteEncodingIndex* index)
    {
        this->index = index;
    }

    void SingleByteDecoder::Initialize(ISingleByteEncodingIndex* index, IStream<Codepoint>* destination)
    {
        this->index = index;
        this->destination = destination;
    }

    void SingleByteDecoder::set_destination(IStream<Codepoint>* destination)
    {
        this->destination = destination;
    }

    void SingleByteDecoder::EmitDecoderError(const char* error)
    {
        char full_error[0x100];
        int result = sprintf_s(full_error, "decoder error(%s)", error);
        if (result == -1)
            throw new Exception("SingleByteDecoder::EmitDecoderError");

        HandleError(full_error);

        Emit(0xFFFD);
    }

    void SingleByteDecoder::EmitDecoderError(byte b)
    {
        char error[0x100];
        int result = sprintf_s(error, "byte=0x%02X", b);
        if (result == -1)
            throw new Exception("SingleByteDecoder::EmitDecoderError");

        EmitDecoderError(error);
    }

    void SingleByteDecoder::Write(const byte* elements, uint32 count)
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
    
        // http://encoding.spec.whatwg.org/#legacy-single-byte-encodings

        uint32 i = 0;

        while (true)
        {
            if (i == count)
                break;

            byte b = elements[i];

            i++;

            if (b >= 0 && b <= 0x7F)
            {
                Emit(b);
                continue;
            }

            Codepoint codepoint = this->index->pointer_to_codepoint(b - 0x80);

            if (codepoint == 0)
            {
                EmitDecoderError(b);
                continue;
            }

            Emit(codepoint);
            continue;
        }
    }

    void SingleByteDecoder::WriteEOF()
    {
        Emit(EOF);
    }

    void SingleByteDecoder::Emit(Codepoint codepoint)
    {
        this->destination->Write(&codepoint, 1);
    }

    void SingleByteDecoderFactory::Initialize(ISingleByteEncodingIndex* index)
    {
        this->index = index;
    }

    void SingleByteDecoderFactory::CreateDecoder(Basic::Ref<IDecoder>* decoder)
    {
        SingleByteDecoder::Ref single_byte_decoder = New<SingleByteDecoder>();
        single_byte_decoder->Initialize(this->index);

        (*decoder) = single_byte_decoder;
    }
}