// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"
#include "Basic.IStream.h"

namespace Http
{
    using namespace Basic;

    class DisconnectBodyFrame : public Frame
    {
    private:
        enum State
        {
            receiving_body_state = Start_State,
            done_state = Succeeded_State,
        };

        std::shared_ptr<IStream<byte> > body_stream;

        virtual EventResult IProcess::consider_event(IEvent* event);

    public:
        DisconnectBodyFrame(std::shared_ptr<IStream<byte> > body_stream);
    };
}