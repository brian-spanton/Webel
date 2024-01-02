// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Service.Types.h"

namespace Service
{
    uint32 TaskCompleteEvent::get_type()
    {
        return EventType::task_complete_event;
    }

    uint32 CharactersCompleteEvent::get_type()
    {
        return EventType::characters_complete_event;
    }
}