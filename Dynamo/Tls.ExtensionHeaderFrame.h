#pragma once

#include "Basic.IProcess.h"
#include "Tls.Types.h"
#include "Tls.NumberFrame.h"

namespace Tls
{
	using namespace Basic;

	class ExtensionHeaderFrame : public Frame
	{
	private:
		enum State
		{
			type_frame_pending_state = Start_State,
			length_frame_pending_state,
			done_state = Succeeded_State,
			type_frame_failed,
			length_frame_failed,
		};

		ExtensionHeader* extension;
		Inline<NumberFrame<ExtensionType> > type_frame;
		Inline<NumberFrame<uint16> > length_frame;

	public:
		typedef Basic::Ref<ExtensionHeaderFrame, IProcess> Ref;

		void Initialize(ExtensionHeader* extension);

		virtual void IProcess::Process(IEvent* event, bool* yield);
	};
}