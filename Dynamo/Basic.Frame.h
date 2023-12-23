#pragma once

#include "Basic.IProcess.h"

namespace Basic
{
	class Frame : public IProcess
	{
	private:
		uint32 state;

	protected:
		static const uint32 Start_State = 0;
		static const uint32 Succeeded_State = 0x10000;

		void switch_to_state(uint32 state);
		void Initialize();

	public:
		uint32 frame_state();

		virtual void IProcess::Process(IEvent* event, bool* yield) = 0;
		virtual void IProcess::Process(IEvent* event);
		virtual bool IProcess::Pending();
		virtual bool IProcess::Succeeded();
		virtual bool IProcess::Failed();

		static void Process(IProcess* frame, IEvent* event);
	};
}