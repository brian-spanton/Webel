// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"

namespace Basic
{
	template <class T>
	__interface IBufferedStream : public IStream<T>
	{
		void Flush();
	};
}