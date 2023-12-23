#pragma once

#include "Basic.IRefCounted.h"

namespace Basic
{
	__interface ICompletion : public IRefCounted
	{
		void CompleteAsync(OVERLAPPED_ENTRY& entry);
	};
}