// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IEncoder.h"

namespace Basic
{
	__interface IEncoderFactory : public IRefCounted
	{
		void CreateEncoder(Ref<IEncoder>* value);
	};
}