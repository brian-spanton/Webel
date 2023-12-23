// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.StateMachine.h"
#include "Basic.IElementConsumer.h"
#include "Basic.IElementSource.h"

namespace Basic
{
	bool HandleError(const char* context);
	ConsumeElementsResult consume_elements(IElementConsumer<byte>* element_consumer, IElementSource<byte>* element_source, StateMachine* state_machine, uint32 fail_state);
}
