#pragma once

#include "Basic.Ref.h"
#include "Basic.Frame.h"

namespace Basic
{
	class CommandFrame : public Frame, public IRefHolder
	{
	private:
		enum State
		{
			word_state = Start_State,
			done_state = Succeeded_State,
		};

		UnicodeString::Ref word;
		std::vector<UnicodeString::Ref>* command; // $$$

	public:
		typedef Basic::Ref<CommandFrame, IProcess> Ref;

		CommandFrame();

		void Initialize(std::vector<UnicodeString::Ref>* command);
		virtual void IProcess::Process(IEvent* event, bool* yield);
	};
}