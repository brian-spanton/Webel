// Copyright © 2013 Brian Spanton

#pragma once

#include "Tls.Globals.h"

namespace Tls
{
	class ResponderIDListFrame : public VectorFrame<OpaqueVector, 0, 0xffff, 2>
	{
	private:
		virtual void GetItemFrame(Item* item, Basic::Ref<IProcess>* value)
		{
			ResponderIDFrame::Ref frame = New<ResponderIDFrame>();
			frame->Initialize(item);
			(*value) = frame;
		}

		virtual void GetItemSerializer(Item* item, Basic::Ref<ISerializable>* value)
		{
			ResponderIDFrame::Ref frame = New<ResponderIDFrame>();
			frame->Initialize(item);
			(*value) = frame;
		}
	};
}