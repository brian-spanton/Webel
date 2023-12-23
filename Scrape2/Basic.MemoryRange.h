// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.IElementConsumer.h"

namespace Basic
{
    class MemoryRange : public ArrayStream<byte>, public IElementConsumer<byte>
    {
    private:
        uint32 expected;
        uint32 received;
        byte* destination;

    public:
        MemoryRange();
        MemoryRange(byte* destination, uint32 expected);

        void reset(byte* destination, uint32 expected);

		ConsumeElementsResult IElementConsumer<byte>::consume_elements(IElementSource<byte>* element_source);

        void IStream<byte>::write_elements(const byte* elements, uint32 count);
        void IStream<byte>::write_element(byte element);
        void IStream<byte>::write_eof();

        bool in_progress();

        uint32 Length();
    };
}