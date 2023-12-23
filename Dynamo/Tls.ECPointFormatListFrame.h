#pragma once

#include "Tls.Globals.h"

namespace Tls
{
	class ECPointFormatListFrame : public VectorFrame<ECPointFormat, 1, 0xff, 1>
	{
	private:
		virtual void GetItemFrame(Item* item, Basic::Ref<IProcess>* value)
		{
			NumberFrame<ECPointFormat>::Ref frame = New<NumberFrame<ECPointFormat> >();
			frame->Initialize(item);
			(*value) = frame;
		}

		virtual void GetItemSerializer(Item* item, Basic::Ref<ISerializable>* value)
		{
			NumberFrame<ECPointFormat>::Ref frame = New<NumberFrame<ECPointFormat> >();
			frame->Initialize(item);
			(*value) = frame;
		}
	};
}