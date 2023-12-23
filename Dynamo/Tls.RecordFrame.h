#pragma once

#include "Basic.IProcess.h"
#include "Basic.ISerializable.h"
#include "Basic.MemoryRange.h"
#include "Tls.NumberFrame.h"
#include "Tls.Types.h"

namespace Tls
{
	using namespace Basic;

	class RecordFrame : public Frame, public ISerializable
	{
	private:
		enum State
		{
			type_frame_pending_state = Start_State,
			version_frame_pending_state,
			length_frame_pending_state,
			fragment_frame_pending_state,
			done_state = Succeeded_State,
			type_frame_failed,
			version_frame_failed,
			length_frame_failed,
			fragment_frame_failed,
		};

		Record* record;
		Inline<NumberFrame<ContentType> > type_frame;
		Inline<NumberFrame<ProtocolVersion> > version_frame;
		Inline<NumberFrame<uint16> > length_frame;
		Inline<MemoryRange> fragment_frame;

	public:
		typedef Basic::Ref<RecordFrame, IProcess> Ref;

		void Initialize(Record* record);

		virtual void IProcess::Process(IEvent* event, bool* yield);

		virtual void ISerializable::SerializeTo(IStream<byte>* stream);
	};
}