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

    ProcessResult Endpoint::consider_event(IEvent* event)
    {
        if (event->get_type() != Http::EventType::accept_complete_event)
        {
            HandleError("unexpected event");
            return ProcessResult::process_result_blocked; // unexpected event
        }

        SpawnListener();
        return ProcessResult::process_result_blocked; // event consumed
    }
}