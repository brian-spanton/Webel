// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.MemoryRange.h"
#include "Basic.Globals.h"
#include "Basic.Event.h"
#include "Basic.Frame.h"

namespace Basic
{
    MemoryRange::MemoryRange() :
        received(0),
        bytes(0),
        count(0)
    {
    }

    MemoryRange::MemoryRange(byte* bytes, uint32 count) :
        received(0),
        bytes(bytes),
        count(count)
    {
    }

    void MemoryRange::reset(byte* bytes, uint32 count)
    {
        this->received = 0;
        this->bytes = bytes;
        this->count = count;
    }

    void MemoryRange::write_elements(const byte* elements, uint32 received)
    {
        uint32 remaining = this->count - this->received;
        if (received > remaining)
            throw FatalError("MemoryRange::write_elements received > remaining");

        CopyMemory(this->bytes + this->received, elements, received);
        this->received += received;
    }

    void MemoryRange::write_element(byte element)
    {
        if (this->count == this->received)
            throw FatalError("MemoryRange::write_elements this->count == this->received");

        this->bytes[this->received] = element;
        this->received += 1;
    }

    void MemoryRange::write_eof()
    {
        HandleError("unexpected eof");
    }

    ProcessResult MemoryRange::consider_event(IEvent* event)
    {
        const byte* elements;
        uint32 useable;

        ProcessResult result = Event::Read(event, this->count - this->received, &elements, &useable);
        if (result == process_result_blocked)
            return ProcessResult::process_result_blocked;

        CopyMemory(this->bytes + this->received, elements, useable);
        this->received += useable;

        return ProcessResult::process_result_ready;
    }

    uint32 MemoryRange::Length()
    {
        return this->received;
    }

    bool MemoryRange::in_progress()
    {
        return this->received < this->count;
    }

    bool MemoryRange::succeeded()
    {
        return this->received == this->count;
    }

    bool MemoryRange::failed()
    {
        return false;
    }
}