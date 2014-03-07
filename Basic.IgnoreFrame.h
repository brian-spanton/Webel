// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"

namespace Basic
{
	class IgnoreFrame : public Frame
	{
	private:
		enum State
		{
			receiving_state = Start_State,
			done_state = Succeeded_State,
		};

		uint32 expected;
		uint32 received;

	public:
		typedef Basic::Ref<IgnoreFrame> Ref;

		void Initialize(uint32 expected);

		virtual void IProcess::Process(IEvent* event, bool* yield);
	};
}