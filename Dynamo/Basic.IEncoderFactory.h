#pragma once

#include "Basic.IEncoder.h"

namespace Basic
{
	__interface IEncoderFactory : public IRefCounted
	{
		void CreateEncoder(Ref<IEncoder>* value);
	};
}