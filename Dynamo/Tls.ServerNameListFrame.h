#pragma once

#include "Tls.ServerNameFrame.h"
#include "Tls.VectorFrame.h"

namespace Tls
{
	class ServerNameListFrame : public VectorFrame<ServerName, 1, 0xffff, 2>
	{
	private:
		virtual void GetItemFrame(Item* item, Basic::Ref<IProcess>* value)
		{
			ServerNameFrame::Ref frame = New<ServerNameFrame>();
			frame->Initialize(item);
			(*value) = frame;
		}

		virtual void GetItemSerializer(Item* item, Basic::Ref<ISerializable>* value)
		{
			ServerNameFrame::Ref frame = New<ServerNameFrame>();
			frame->Initialize(item);
			(*value) = frame;
		}
	};
}