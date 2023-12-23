#pragma once

#include "Basic.IProcess.h"
#include "Tls.Types.h"
#include "Tls.NumberFrame.h"

namespace Tls
{
	using namespace Basic;

	class AlertFrame : public Frame, public ISerializable
	{
	private:
		enum State
		{
			level_frame_pending_state = Start_State,
			description_frame_pending_state,
			done_state = Succeeded_State,
		};

		Alert* alert;
		Inline<NumberFrame<AlertLevel> > level_frame;
		Inline<NumberFrame<AlertDescription> > description_frame;

	public:
		typedef Basic::Ref<AlertFrame, IProcess> Ref;

		void Initialize(Alert* alert);
		virtual void IProcess::Process(IEvent* event, bool* yield);
		virtual void ISerializable::SerializeTo(IStream<byte>* stream);
	};
}
