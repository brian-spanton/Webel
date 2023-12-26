// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Tls.Types.h"
#include "Basic.MemoryRange.h"

namespace Tls
{
    using namespace Basic;

    class RandomFrame : public Frame
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
        NumberFrame<uint32> time_frame;
        MemoryRange bytes_frame;

        virtual EventResult IProcess::consider_event(IEvent* event);

    public:
        RandomFrame(Random* random);
    };
}

namespace Basic
{
    template <>
    struct __declspec(novtable) serialize<Tls::Random>
    {
        void operator()(const Tls::Random* value, IStream<byte>* stream) const
        {
            serialize<uint32>()(&value->gmt_unix_time, stream);
            serialize<const byte[28]>()(&value->random_bytes, stream);
        }
    };
}
