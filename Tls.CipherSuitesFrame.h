// Copyright © 2013 Brian Spanton

#pragma once

#include "Tls.VectorFrame.h"
#include "Tls.NumberFrame.h"

namespace Tls
{
	class CipherSuitesFrame : public VectorFrame<CipherSuite, 2, 0xfffe, 2>
	{
	private:
		virtual void GetItemFrame(Item* item, Basic::Ref<IProcess>* value)
		{
			NumberFrame<CipherSuite>::Ref frame = New<NumberFrame<CipherSuite> >();
			frame->Initialize(item);
			(*value) = frame;
		}

		virtual void GetItemSerializer(Item* item, Basic::Ref<ISerializable>* value)
		{
			NumberFrame<CipherSuite>::Ref frame = New<NumberFrame<CipherSuite> >();
			frame->Initialize(item);
			(*value) = frame;
		}
	};
}