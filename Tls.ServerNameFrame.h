// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Basic.ISerializable.h"
#include "Tls.Types.h"
#include "Tls.NumberFrame.h"
#include "Tls.VectorFrames.h"

namespace Tls
{
	using namespace Basic;

	class ServerNameFrame : public Frame, public ISerializable
	{
	private:
		enum State
		{
			start_state = Start_State,
			type_state,
			name_state,
			done_state = Succeeded_State,
			type_frame_failed,
			name_frame_failed,
		};

		ServerName* serverName;
		Inline<NumberFrame<NameType> > type_frame;
		Inline<HostNameFrame> name_frame;

	public:
		typedef Basic::Ref<ServerNameFrame, IProcess> Ref;

		void Initialize(ServerName* serverName);
		virtual void IProcess::Process(IEvent* event, bool* yield);
		virtual void ISerializable::SerializeTo(IStream<byte>* stream);
	};
}