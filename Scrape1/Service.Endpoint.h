// Copyright � 2014 Brian Spanton

#pragma once

#include "Basic.ListenSocket.h"
#include "Basic.Frame.h"

namespace Service
{
    using namespace Basic;

    class Endpoint : public Frame
    {
    protected:
        std::shared_ptr<Basic::ListenSocket> listener;

        virtual void IProcess::consider_event(IEvent* event);

    public:
        Endpoint(ListenSocket::Face face, short port);

        void SpawnListeners(uint16 count);

        virtual void SpawnListener() = 0;
    };
}