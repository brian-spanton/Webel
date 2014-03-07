// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.CommandFrame.h"
#include "Basic.TextWriter.h"
#include "Basic.AsyncBytes.h"
#include "Basic.Ref.h"
#include "Basic.Event.h"

namespace Basic
{
	CommandFrame::CommandFrame()
	{
		this->word.SetHolder(this);
	}

	void CommandFrame::Initialize(std::vector<UnicodeString::Ref>* command)
	{
		__super::Initialize();
		this->command = command;
		this->word = New<UnicodeString>();
	}

	void CommandFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::word_state:
			{
				Codepoint b;
				if (!Event::ReadNext(event, &b, yield))
					return;

				if (b == ' ')
				{
					this->command->push_back(this->word);
					this->word = New<UnicodeString>();
				}
				else if (b == '\r')
				{
					this->command->push_back(this->word);
					switch_to_state(State::done_state);
				}
				else
				{
					this->word->push_back(b);
				}
			}
			break;

		default:
			throw new Exception("CommandFrame::Process unexpected state");
		}
	}
}