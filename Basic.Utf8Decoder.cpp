// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Utf8Decoder.h"
#include "Basic.Globals.h"

namespace Basic
{
	void Utf8Decoder::set_destination(IStream<Codepoint>* destination)
	{
		this->destination = destination;

		// http://encoding.spec.whatwg.org/#the-encoding
		this->codepoint = 0;
		this->seen = 0;
		this->needed = 0;
		this->lower_bound = 0x80;
		this->upper_bound = 0xBF;
	}

	void Utf8Decoder::EmitDecoderError(const char* error)
	{
		char full_error[0x100];
		int result = sprintf_s(full_error, "Utf8Decoder::EmitDecoderError needed=%d seen=%d %hs", this->needed, this->seen, error);
		if (result == -1)
			throw new Exception("sprintf_s");

		HandleError(full_error);

		Emit(0xFFFD);
	}

	void Utf8Decoder::EmitDecoderError(byte b)
	{
		char error[0x100];
		sprintf_s(error, "byte=0x%02X %d", b, b);

		EmitDecoderError(error);
	}

	void Utf8Decoder::Write(const byte* elements, uint32 count)
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
	
		// http://encoding.spec.whatwg.org/#utf-8-decoder

		uint32 i = 0;

		while (true)
		{
			if (i == count)
				break;

			byte b = elements[i];

			i++;

			if (this->needed == 0)
			{
				if (b >= 0 && b <= 0x7F)
				{
					Emit(b);
					continue;
				}
				else if (b >= 0xC2 && b <= 0xDF)
				{
					this->needed = 1;
					this->codepoint = b - 0xC0;
				}
				else if (b >= 0xE0 && b <= 0xEF)
				{
					if (b == 0xE0)
						this->lower_bound = 0xA0;
					else if (b == 0xED)
						this->upper_bound = 0x9F;

					this->needed = 2;
					this->codepoint = b - 0xE0;
				}
				else if (b >= 0xF0 && b <= 0xF4)
				{
					if (b == 0xF0)
						this->lower_bound = 0x90;
					else if (b == 0xF4)
						this->upper_bound = 0x8F;

					this->needed = 3;
					this->codepoint = b - 0xF0;
				}
				else
				{
					EmitDecoderError(b);
					continue;
				}

				// (byte is in the range 0xC2 to 0xF4)
				this->codepoint <<= 6 * this->needed;
				continue;
			}

			if (!(b >= this->lower_bound && b <= this->upper_bound))
			{
				this->codepoint = 0;
				this->needed = 0;
				this->seen = 0;
				this->lower_bound = 0x80;
				this->upper_bound = 0xBF;

				i--;

				EmitDecoderError(b);
				continue;
			}

			this->lower_bound = 0x80;
			this->upper_bound = 0xBF;
			this->seen++;
			this->codepoint += (b - 0x80) << (6 * (this->needed - this->seen));

			if (this->seen != this->needed)
				continue;

			Codepoint codepoint = this->codepoint;

			this->codepoint = 0;
			this->needed = 0;
			this->seen = 0;

			Emit(codepoint);
			continue;
		}
	}

	void Utf8Decoder::WriteEOF()
	{
		if (this->needed != 0)
		{
			this->needed = 0;

			EmitDecoderError("end of stream with needed != 0");
		}

		Emit(EOF);
	}

	void Utf8Decoder::Emit(Codepoint codepoint)
	{
		this->destination->Write(&codepoint, 1);
	}

	void Utf8DecoderFactory::CreateDecoder(Basic::Ref<IDecoder>* decoder)
	{
		(*decoder) = New<Utf8Decoder>();
	}
}