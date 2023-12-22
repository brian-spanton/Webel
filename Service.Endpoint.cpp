// Copyright © 2014 Brian Spanton

#include "stdafx.h"
#include "Service.Endpoint.h"
#include "Http.Types.h"

namespace Service
{
    using namespace Basic;

    Endpoint::Endpoint(ListenSocket::Face face, short port) :
        listener(std::make_shared<Basic::ListenSocket>(face, port))
    {
    }

    void Endpoint::SpawnListeners(uint16 count)
    {
        for (uint16 i = 0; i < count; i++)
        {
            SpawnListener();
        }
    }

    EventResult Endpoint::consider_event(IEvent* event)
    {
        if (event->get_type() != Http::EventType::accept_complete_event)
        {
            HandleError("unexpected event");
            return EventResult::event_result_yield; // unexpected event
        }

        SpawnListener();
        return EventResult::event_result_yield; // event consumed
    }
}