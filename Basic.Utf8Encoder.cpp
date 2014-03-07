// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Utf8Encoder.h"
#include "Basic.Globals.h"

namespace Basic
{
	Utf8Encoder::Utf8Encoder()
		: error_replacement_byte(0x7F) // $ is this a good default error_replacement_byte char?
	{
	}

	void Utf8Encoder::set_destination(IStream<byte>* destination)
	{
		this->destination = destination;
	}

	void Utf8Encoder::set_error_replacement_byte(byte error_replacement_byte)
	{
		this->error_replacement_byte = error_replacement_byte;
	}

	void Utf8Encoder::EncoderError(Codepoint codepoint)
	{
		char error[0x100];
		int result = sprintf_s(error, "Utf8Encoder::EncoderError codepoint=0x%04X", codepoint);
		if (result == -1)
			throw new Exception("sprintf_s");

		HandleError(error);
	}

	void Utf8Encoder::Write(const Codepoint* elements, uint32 count)
	{
		// From http://encoding.spec.whatwg.org/#encodings
		// An encoder algorithm takes a code point stream and emits a byte stream. It fails when a code point is passed for
		// which it does not have a corresponding byte (sequence). Analogously to a decoder, it has a code point pointer and
		// encoder error. An encoder must be invoked again when the word continue is used, or when one or more bytes are emitted
		// of which none is the EOF byte. Unless stated otherwise, when an encoder error is emitted the encoder terminates. 
		//
		//     Note: HTML forms and URLs require non-terminating encoders and have therefore special handling whenever an 
		//     encoder error is reached. Instead of terminating the encoder one or more code points in the range U+0000 to U+007F
		//     are inserted into the code point stream at code point pointer after encoder error is emitted. 
	
		// http://encoding.spec.whatwg.org/#utf-8-encoder

		uint32 i = 0;

		while (true)
		{
			if (i == count)
				break;

			Codepoint codepoint = elements[i];

			if (codepoint == EOF)
			{
				this->destination->WriteEOF();
				return;
			}

			i++;

			if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
			{
				EncoderError(codepoint);
				Emit(this->error_replacement_byte);
				continue;
			}

			if (codepoint >= 0x0000 && codepoint <= 0x007F)
			{
				Emit((byte)codepoint);
				continue;
			}

			byte count;
			byte offset;

			if (codepoint >= 0x0080 && codepoint <= 0x07FF)
			{
				count = 1;
				offset = 0xC0;
			}
			else if (codepoint >= 0x0800 && codepoint <= 0xFFFF)
			{
				count = 2;
				offset = 0xE0;
			}
			else if (codepoint >= 0x10000 && codepoint <= 0x10FFFF)
			{
				count = 3;
				offset = 0xF0;
			}

			byte b = (byte)((codepoint >> (6 * count)) + offset);
			Emit(b);

			while (count > 0)
			{
				Codepoint temp = codepoint >> (6 * (count - 1));
				b = (byte)(0x80 + (temp % 64));
				Emit(b);

				count --;
			}
		}
	}

	void Utf8Encoder::WriteEOF()
	{
		this->destination->WriteEOF();
	}

	void Utf8Encoder::Emit(byte b)
	{
		this->destination->Write(&b, 1);
	}

	void Utf8EncoderFactory::CreateEncoder(Basic::Ref<IEncoder>* encoder)
	{
		(*encoder) = New<Utf8Encoder>();
	}
}