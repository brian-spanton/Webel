#include "stdafx.h"
#include "Dynamo.Types.h"

namespace Dynamo
{
	uint32 TaskCompleteEvent::get_type()
	{
		return EventType::task_complete_event;
	}
}