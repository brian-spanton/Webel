// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.MemoryRange.h"
#include "Basic.Globals.h"

namespace Basic
{
    MemoryRange::MemoryRange() :
        received(0),
        destination(0),
        expected(0)
    {
    }

    MemoryRange::MemoryRange(byte* destination, uint32 expected) :
        received(0),
        destination(destination),
        expected(expected)
    {
    }

    void MemoryRange::reset(byte* destination, uint32 expected)
    {
        this->received = 0;
        this->destination = destination;
        this->expected = expected;
    }

	ConsumeElementsResult MemoryRange::consume_elements(IElementSource<byte>* element_source)
	{
		uint32 remaining = this->expected - this->received;

		const byte* elements;
		uint32 count;

		element_source->Read(remaining, &elements, &count);

		CopyMemory(this->destination + this->received, elements, count);
		this->received += count;

		if (in_progress())
			return ConsumeElementsResult::in_progress;

		return ConsumeElementsResult::succeeded;
	}

    void MemoryRange::write_elements(const byte* elements, uint32 count)
    {
        uint32 remaining = this->expected - this->received;

        if (count > remaining)
            throw FatalError("MemoryRange::write_elements received > remaining");

        CopyMemory(this->destination + this->received, elements, count);
        this->received += count;
    }

    uint32 MemoryRange::Length()
    {
        return this->received;
    }

    bool MemoryRange::in_progress()
    {
        return this->received < this->expected;
    }
}