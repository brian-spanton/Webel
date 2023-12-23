#pragma once

namespace Basic
{
	__interface ISingleByteEncodingIndex : public IRefCounted
	{
		Codepoint byte_to_codepoint(byte b);
		byte codepoint_to_byte(Codepoint c);
	};
}