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

    IoCompletionEvent::IoCompletionEvent(std::shared_ptr<void> context, uint32 count, uint32 error) :
        context(context),
        count(count),
        error(error)
    {
    }

    uint32 IoCompletionEvent::get_type()
    {
        return EventType::io_completion_event;
    }
}