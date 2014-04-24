// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Tls.Types.h"
#include "Basic.MemoryRange.h"
#include "Tls.NumberFrame.h"

namespace Tls
{
    using namespace Basic;

    class RandomFrame : public Frame, public ISerializable
    {
    private:
        enum State
        {
            time_frame_pending_state = Start_State,
            bytes_frame_pending_state,
            done_state = Succeeded_State,
            time_frame_failed,
            bytes_frame_failed,
        };

        Random* random;
        Inline<NumberFrame<uint32> > time_frame;
        Inline<MemoryRange> bytes_frame;

    public:
        typedef Basic::Ref<RandomFrame, IProcess> Ref;

        void Initialize(Random* random);
        virtual void IProcess::Process(IEvent* event, bool* yield);
        virtual void ISerializable::SerializeTo(IStream<byte>* stream);
    };
}
