// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IRefCounted.h"

namespace Basic
{
	template <class character_type>
	__interface INumberStream : public IRefCounted
	{
		bool WriteDigit(character_type digit);
		uint8 get_digit_count();
	};
}