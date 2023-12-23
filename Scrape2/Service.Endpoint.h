// Copyright © 2014 Brian Spanton

#pragma once

#include "Basic.ListenSocket.h"

namespace Service
{
    using namespace Basic;

    class Endpoint : public StateMachine
    {
    protected:
        std::shared_ptr<Basic::ListenSocket> listener;

        void consider_event(void* event);

    public:
        Endpoint(ListenSocket::Face face, short port);

        void SpawnListeners(uint16 count);

        virtual void SpawnListener() = 0;
    };
}