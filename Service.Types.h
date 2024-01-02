// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Event.h"
#include "Basic.Uri.h"
#include "Basic.SuffixArray.h"
#include "Json.Types.h"

namespace Service
{
    enum EventType
    {
        task_complete_event = 0x4000,
        characters_complete_event,
    };

    struct CookieContextualizedEvent : public Basic::IEvent
    {
        Basic::ByteStringRef cookie;

        virtual uint32 Basic::IEvent::get_type() = 0;
    };

    struct TaskCompleteEvent : public CookieContextualizedEvent
    {
        virtual uint32 Basic::IEvent::get_type();
    };

    struct CharactersCompleteEvent : public CookieContextualizedEvent
    {
        virtual uint32 Basic::IEvent::get_type();
    };

    typedef Basic::SuffixArray<std::shared_ptr<Json::Object> > Index;
}