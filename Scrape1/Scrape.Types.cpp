// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Scrape.Types.h"

namespace Scrape
{
    uint32 TaskCompleteEvent::get_type()
    {
        return EventType::task_complete_event;
    }
}