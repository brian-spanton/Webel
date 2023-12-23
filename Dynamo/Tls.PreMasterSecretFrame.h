#pragma once

#include "Basic.IProcess.h"
#include "Basic.ISerializable.h"
#include "Basic.MemoryRange.h"
#include "Tls.Types.h"
#include "Tls.NumberFrame.h"

namespace Tls
{
	using namespace Basic;

	class PreMasterSecretFrame : public Frame, public ISerializable
	{
	private:
		enum State
		{
			version_frame_pending_state = Start_State,
			random_frame_pending_state,
			done_state = Succeeded_State,
			version_frame_failed,
			random_frame_failed,
		};

		PreMasterSecret* pre_master_secret;
		Inline<NumberFrame<uint16> > version_frame;
		Inline<MemoryRange> random_frame;

	public:
		typedef Basic::Ref<PreMasterSecretFrame, IProcess> Ref;

		void Initialize(PreMasterSecret* pre_master_secret);
		virtual void IProcess::Process(IEvent* event, bool* yield);
		virtual void ISerializable::SerializeTo(IStream<byte>* stream);
	};
}