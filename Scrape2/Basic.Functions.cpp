// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Functions.h"
#include "Basic.Globals.h"

namespace Basic
{
    bool HandleError(const char* context)
    {
        return Basic::globals->HandleError(context, 0);
    }

	ConsumeElementsResult consume_elements(IElementConsumer<byte>* element_consumer, IElementSource<byte>* element_source, StateMachine* state_machine, uint32 fail_state)
	{
		ConsumeElementsResult result = element_consumer->consume_elements(element_source);

		if (result == ConsumeElementsResult::failed)
		{
			state_machine->switch_to_state(fail_state);
		}

		return result;
	}
}
