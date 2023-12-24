// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.FrameStream.h"

namespace Basic
{
    void ProcessStream<byte>::write_elements(const byte* elements, uint32 count)
    {
        this->element_source.Initialize(elements, count);

        ReceivedBytesEvent event;
        event.Initialize(&this->element_source);

        produce_event(this->process.get(), &event);
    }

    void ProcessStream<Codepoint>::write_elements(const Codepoint* elements, uint32 count)
    {
        this->element_source.Initialize(elements, count);

        ReceivedCodepointsEvent event;
        event.Initialize(&this->element_source);

        produce_event(this->process.get(), &event);
    }
}