// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Basic.IStream.h"
#include "Basic.ISerializable.h"

namespace Basic
{
	class MemoryRange : public IProcess, public IStream<byte>, public ISerializable
	{
	private:
		uint32 count;
		uint32 received;
		byte* bytes;

	public:
		typedef Basic::Ref<MemoryRange, IProcess> Ref;

		MemoryRange();

		void Initialize(byte* bytes, uint32 count);

		virtual void IStream<byte>::Write(const byte* elements, uint32 received);
		virtual void IStream<byte>::WriteEOF();

		virtual void IProcess::Process(IEvent* event, bool* yield);
		virtual void IProcess::Process(IEvent* event);
		virtual bool IProcess::Pending();
		virtual bool IProcess::Succeeded();
		virtual bool IProcess::Failed();

		virtual void ISerializable::SerializeTo(IStream<byte>* stream);

		uint32 Length();
	};
}