// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.IProcess.h"
#include "Basic.ElementSource.h"
#include "Basic.Event.h"

namespace Basic
{
    // the inverse of a StreamFrame, the point of ProcessStream is to present any IProcess as an IStream.
    // the only IProcess types are Frame and MemoryRange, and Memory range natively implements IStream.
    // thus it serves to make a Frame compatible with IStream inputs.
    template <class element_type>
    class ProcessStream : public ArrayStream<element_type>
    {
    private:
        Basic::ElementSource<element_type> element_source;
        std::shared_ptr<IProcess> process;

    public:
        ProcessStream(std::shared_ptr<IProcess> process)
        {
            this->process = process;
        }

        virtual void IStream<element_type>::write_elements(const element_type* elements, uint32 count);

        virtual void IStream<element_type>::write_eof()
        {
            ElementStreamEndingEvent event;
            produce_event(this->process.get(), &event);
        }

        static bool write_elements_to_process(std::shared_ptr<IProcess> process, const element_type* bytes, uint32 count)
        {
            ProcessStream<element_type> stream(process);

            stream.write_elements(bytes, count);
            stream.write_eof();

            return process->succeeded();
        }
    };
}