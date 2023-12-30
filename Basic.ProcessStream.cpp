// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.ProcessStream.h"

namespace Basic
{
    void ProcessStream<byte>::write_elements(const byte* elements, uint32 count)
    {
        this->element_source.Initialize(elements, count);

        ReceivedBytesEvent event;
        event.Initialize(&this->element_source);

        auto result = delegate_event(this->process.get(), &event);

        if (this->process->failed())
            throw FatalError("ProcessStream<byte>::write_elements { this->process->failed() }");

        if (!this->element_source.Exhausted())
            throw FatalError("ProcessStream<byte>::write_elements { !this->element_source.Exhausted() }");
    }

    void ProcessStream<Codepoint>::write_elements(const Codepoint* elements, uint32 count)
    {
        this->element_source.Initialize(elements, count);

        ReceivedCodepointsEvent event;
        event.Initialize(&this->element_source);

        auto result = delegate_event(this->process.get(), &event);

        if (this->process->failed())
            throw FatalError("ProcessStream<Codepoint>::write_elements { this->process->failed() }");

        if (!this->element_source.Exhausted())
            throw FatalError("ProcessStream<Codepoint>::write_elements { !this->element_source.Exhausted() }");
    }
}