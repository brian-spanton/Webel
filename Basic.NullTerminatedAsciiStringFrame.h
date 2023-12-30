// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"
#include "Basic.SingleByteEncodingIndex.h"

namespace Basic
{
    class NullTerminatedAsciiStringFrame : public Frame
    {
    private:
        enum State
        {
            receiving_state = Start_State,
            done_state = Succeeded_State,
        };

        SingleByteDecoder decoder;

    public:
        NullTerminatedAsciiStringFrame(Basic::IStream<Codepoint>* destination);

        virtual EventResult IProcess::consider_event(IEvent* event);
    };
}