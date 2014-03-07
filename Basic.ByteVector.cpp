// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.ByteVector.h"

namespace Basic
{
	void ByteVector::Write(const byte* elements, uint32 count)
	{
		insert(end(), elements, elements + count);
	}

	void ByteVector::WriteEOF()
	{
	}

	void ByteVector::SerializeTo(IStream<byte>* stream)
	{
		if (size() > 0)
		{
			stream->Write(FirstElement(), size());
		}
	}

	byte* ByteVector::FirstElement()
	{
		return &(*this)[0];
	}
}