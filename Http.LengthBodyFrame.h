// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"
#include "Basic.IStream.h"

namespace Http
{
    using namespace Basic;

    class LengthBodyFrame : public Frame
    {
    private:
        enum State
        {
            receiving_body_state = Start_State,
            done_state = Succeeded_State,
        };

        std::shared_ptr<IStream<byte> > body_stream;
        uint32 bytes_expected;
        uint32 bytes_received;

        virtual event_result IProcess::consider_event(IEvent* event);

    public:
        LengthBodyFrame(std::shared_ptr<IStream<byte> > body_stream);
        LengthBodyFrame(std::shared_ptr<IStream<byte> > body_stream, uint32 bytes_expected);

        void reset(uint32 bytes_expected);
    };
}