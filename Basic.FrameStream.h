// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.IProcess.h"
#include "Basic.ElementSource.h"
#include "Basic.Event.h"

namespace Basic
{
    template <class T>
    class FrameStream : public IStream<T>
    {
    private:
        Inline<Basic::ElementSource<T> > element_source;
        Basic::Ref<IProcess> frame; // REF

    public:
        typedef Basic::Ref<FrameStream> Ref;

        void Initialize(IProcess* frame)
        {
            this->frame = frame;
        }

        virtual void IStream<T>::Write(const T* elements, uint32 count);

        virtual void IStream<T>::WriteEOF()
        {
            ElementStreamEndingEvent event;
            this->frame->Process(&event);
        }

        static bool Process(IProcess* frame, const T* bytes, uint32 count)
        {
            Inline<FrameStream<T> > protocol;
            protocol.Initialize(frame);

            protocol.Write(bytes, count);
            protocol.WriteEOF();

            return frame->Succeeded();
        }
    };
}