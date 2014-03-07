// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.ISerializable.h"

namespace Basic
{
	class ByteVector : public std::vector<byte>, public IStream<byte>, public ISerializable
	{
	public:
		typedef Basic::Ref<ByteVector, IStream<byte> > Ref;

		virtual void IStream<byte>::Write(const byte* elements, uint32 count);
		virtual void IStream<byte>::WriteEOF();
		virtual void ISerializable::SerializeTo(IStream<byte>* stream);
		byte* FirstElement();
	};
}