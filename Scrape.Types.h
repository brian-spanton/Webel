// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Event.h"
#include "Basic.Uri.h"
#include "Basic.SuffixArray.h"
#include "Json.Types.h"

namespace Scrape
{
    enum EventType
    {
        task_complete_event = 0x3000,
    };

    struct TaskCompleteEvent : public Basic::ContextualizedEvent
    {
        virtual uint32 get_type();
    };

    typedef Basic::SuffixArray<std::shared_ptr<Json::Object> > Index;
}