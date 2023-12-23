#pragma once

#include "Basic.IRefCounted.h"
#include "Basic.IStream.h"
#include "Basic.IBufferedStream.h"

namespace Basic
{
	__interface IProtocolFactory : public IRefCounted
	{
		void CreateProtocol(IBufferedStream<byte>* peer, Ref<IStream<byte> >* protocol);
	};
}