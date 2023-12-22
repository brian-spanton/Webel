// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.IProcess.h"
#include "Basic.ElementSource.h"
#include "Basic.Event.h"

namespace Basic
{
    // $$ get rid of this class?
    template <class element_type>
    class FrameStream : public ArrayStream<element_type>
    {
    private:
        Basic::ElementSource<element_type> element_source;
        IProcess* frame = 0;

    public:
        void Initialize(IProcess* frame)
        {
            this->frame = frame;
        }

        virtual void IStream<element_type>::write_elements(const element_type* elements, uint32 count);

        virtual void IStream<element_type>::write_eof()
        {
            ElementStreamEndingEvent event;
            produce_event(this->frame, &event);
        }

        static bool handle_event(IProcess* frame, const element_type* bytes, uint32 count)
        {
            FrameStream<element_type> protocol;
            protocol.Initialize(frame);

            protocol.write_elements(bytes, count);
            protocol.write_eof();

            return frame->succeeded();
        }
    };
}