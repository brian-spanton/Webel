#include "stdafx.h"
#include "Html.Types.h"

namespace Html
{
	uint32 CharactersCompleteEvent::get_type()
	{
		return EventType::characters_complete_event;
	}
}