// Copyright © 2013 Brian Spanton

#pragma once

#include "Tls.Types.h"
#include "Basic.MemoryRange.h"
#include "Basic.StateMachine.h"

namespace Tls
{
    using namespace Basic;

    class RandomFrame : public StateMachine, public IElementConsumer<byte>
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

    public:
        RandomFrame(Random* random);

		ConsumeElementsResult IElementConsumer<byte>::consume_elements(IElementSource<byte>* element_source);
	};

    template <>
    struct __declspec(novtable) serialize<Random>
    {
        void operator()(const Random* value, IStream<byte>* stream) const
        {
            serialize<uint32>()(&value->gmt_unix_time, stream);
            serialize<const byte[28]>()(&value->random_bytes, stream);
        }
    };
}
