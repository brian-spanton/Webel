// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"
#include "Basic.IStream.h"
#include "Basic.IEvent.h"
#include "Http.BodyFrame.h"

namespace Http
{
    using namespace Basic;

    // this class handles an http body terminated by transport disconnect
    class DisconnectBodyFrame : public BodyFrame
    {
    private:
        enum State
        {
            receiving_body_state = Start_State,
            done_state = Succeeded_State,
        };

        virtual ProcessResult IProcess::consider_event(IEvent* event);

    public:
        DisconnectBodyFrame(std::shared_ptr<IStream<byte> > body_stream);
    };
}