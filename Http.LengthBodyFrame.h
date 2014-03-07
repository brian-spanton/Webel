// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"
#include "Basic.IStream.h"

namespace Http
{
	using namespace Basic;

	class LengthBodyFrame : public Frame
	{
	private:
		enum State
		{
			receiving_body_state = Start_State,
			done_state = Succeeded_State,
		};

		Ref<IStream<byte> > body_stream; // REF
		uint32 bytes_expected;
		uint32 bytes_received;

	public:
		void Initialize(IStream<byte>* body_stream, uint32 bytes_expected);

		virtual void IProcess::Process(IEvent* event, bool* yield);
	};
}