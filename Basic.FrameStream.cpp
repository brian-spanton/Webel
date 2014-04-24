// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.FrameStream.h"

namespace Basic
{
    void FrameStream<byte>::Write(const byte* elements, uint32 count)
    {
        this->element_source.Initialize(elements, count);

        ReadyForReadBytesEvent event;
        event.Initialize(&this->element_source);

        this->frame->Process(&event);
    }

    void FrameStream<Codepoint>::Write(const Codepoint* elements, uint32 count)
    {
        this->element_source.Initialize(elements, count);

        ReadyForReadCodepointsEvent event;
        event.Initialize(&this->element_source);

        this->frame->Process(&event);
    }
}