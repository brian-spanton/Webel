#pragma once

#include "Tls.Globals.h"

namespace Tls
{
	class EllipticCurveListFrame : public VectorFrame<NamedCurve, 1, 0xffff, 2>
	{
	private:
		virtual void GetItemFrame(Item* item, Basic::Ref<IProcess>* value)
		{
			NumberFrame<NamedCurve>::Ref frame = New<NumberFrame<NamedCurve> >();
			frame->Initialize(item);
			(*value) = frame;
		}

		virtual void GetItemSerializer(Item* item, Basic::Ref<ISerializable>* value)
		{
			NumberFrame<NamedCurve>::Ref frame = New<NumberFrame<NamedCurve> >();
			frame->Initialize(item);
			(*value) = frame;
		}
	};
}