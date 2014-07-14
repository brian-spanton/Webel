// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.FrameStream.h"

namespace Basic
{
    void FrameStream<byte>::write_elements(const byte* elements, uint32 count)
    {
        this->element_source.Initialize(elements, count);

        ReadyForReadBytesEvent event;
        event.Initialize(&this->element_source);

        produce_event(this->frame, &event);
    }

    void FrameStream<Codepoint>::write_elements(const Codepoint* elements, uint32 count)
    {
        this->element_source.Initialize(elements, count);

        ReadyForReadCodepointsEvent event;
        event.Initialize(&this->element_source);

        produce_event(this->frame, &event);
    }
}