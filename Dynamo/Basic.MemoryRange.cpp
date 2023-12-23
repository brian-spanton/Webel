#include "stdafx.h"
#include "Basic.MemoryRange.h"
#include "Basic.Globals.h"
#include "Basic.Event.h"
#include "Basic.Frame.h"

namespace Basic
{
	MemoryRange::MemoryRange() :
		bytes(0),
		count(0),
		received(0)
	{
	}

	void MemoryRange::Initialize(byte* bytes, uint32 count)
	{
		this->bytes = bytes;
		this->count = count;
		this->received = 0;
	}

	void MemoryRange::Write(const byte* elements, uint32 received)
	{
		uint32 remaining = this->count - this->received;
		if (received > remaining)
			throw new Exception("MemoryRange::Write received > remaining");

		CopyMemory(this->bytes + this->received, elements, received);
		this->received += received;
	}

	void MemoryRange::WriteEOF()
	{
	}

	void MemoryRange::Process(IEvent* event, bool* yield)
	{
		const byte* elements;
		uint32 useable;

		if(!Event::Read(event, this->count - this->received, &elements, &useable, yield))
			return;

		CopyMemory(this->bytes + this->received, elements, useable);
		this->received += useable;

		if (this->received < this->count)
			(*yield) = true;
	}

	void MemoryRange::SerializeTo(IStream<byte>* stream)
	{
		stream->Write(this->bytes, this->count);
	}

	uint32 MemoryRange::Length()
	{
		return this->received;
	}

	void MemoryRange::Process(IEvent* event)
	{
		Frame::Process(this, event);
	}

	bool MemoryRange::Pending()
	{
		return this->received < this->count;
	}

	bool MemoryRange::Succeeded()
	{
		return this->received == this->count;
	}

	bool MemoryRange::Failed()
	{
		return false;
	}
}