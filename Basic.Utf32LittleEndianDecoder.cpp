// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Utf32LittleEndianDecoder.h"
#include "Basic.Globals.h"

namespace Basic
{
    void Utf32LittleEndianDecoder::set_destination(IStream<Codepoint>* destination)
    {
        this->destination = destination;
        this->received = 0;
    }

    void Utf32LittleEndianDecoder::write_elements(const byte* elements, uint32 count)
    {
        for (const byte* element = elements; element != elements + count; element++)
        {
            write_element(*element);
        }
    }

    void Utf32LittleEndianDecoder::write_element(byte b)
    {
        byte* value_bytes = reinterpret_cast<byte*>(&this->codepoint);
        int index = this->received;
        value_bytes[index] = b;

        this->received++;

        if (this->received == 4)
        {
            this->received = 0;
            Emit(this->codepoint);
        }
    }

    void Utf32LittleEndianDecoder::write_eof()
    {
        if (this->received != 0)
        {
            HandleError("end of stream with received != 0");
        }

        // I think eof is for the end of the bytes to encode, and should not propagate to the destination
        // because it might not at all be the last thing sent to the destination
    }

    void Utf32LittleEndianDecoder::Emit(Codepoint codepoint)
    {
        this->destination->write_element(codepoint);
    }

    void Utf32LittleEndianDecoderFactory::CreateDecoder(std::shared_ptr<IDecoder>* decoder)
    {
        (*decoder) = std::make_shared<Utf32LittleEndianDecoder>();
    }
}