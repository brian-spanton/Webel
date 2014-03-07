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

	void Utf32LittleEndianDecoder::Write(const byte* elements, uint32 count)
	{
		for (uint32 i = 0; i != count; i++)
		{
			byte b = elements[i];

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
	}

	void Utf32LittleEndianDecoder::WriteEOF()
	{
		if (this->received != 0)
		{
			HandleError("end of stream with received != 0");
		}

		Emit(EOF);
	}

	void Utf32LittleEndianDecoder::Emit(Codepoint codepoint)
	{
		this->destination->Write(&codepoint, 1);
	}

	void Utf32LittleEndianDecoderFactory::CreateDecoder(Basic::Ref<IDecoder>* decoder)
	{
		(*decoder) = New<Utf32LittleEndianDecoder>();
	}
}