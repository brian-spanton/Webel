// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"

namespace Basic
{
	__interface IEncoder : public IStream<Codepoint>
	{
		void set_destination(IStream<byte>* destination);
		void set_error_replacement_byte(byte b);
	};
}