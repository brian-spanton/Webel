#pragma once

#include "Basic.IRefCounted.h"

namespace Basic
{
	template <class T>
	__interface IStream : public IRefCounted
	{
		void Write(const T* elements, uint32 count);
		void WriteEOF();
	};
}