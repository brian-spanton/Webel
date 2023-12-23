#pragma once

#include "Basic.IStream.h"

namespace Basic
{
	__interface IDecoder : public IStream<byte>
	{
		void set_destination(IStream<Codepoint>* destination);
	};
}