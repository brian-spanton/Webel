// Copyright © 2013 Brian Spanton

#pragma once

#include "Tls.Types.h"
#include "Basic.StateMachine.h"

namespace Tls
{
    using namespace Basic;

    class HeartbeatExtensionFrame : public StateMachine, public IElementConsumer<byte>
    {
    private:
        enum State
        {
            mode_frame_pending_state = Start_State,
            done_state = Succeeded_State,
            mode_frame_failed,
        };

        HeartbeatExtension* heartbeat_extension;
        NumberFrame<HeartbeatMode> mode_frame;

    public:
        HeartbeatExtensionFrame(HeartbeatExtension* heartbeat_extension);

		ConsumeElementsResult IElementConsumer<byte>::consume_elements(IElementSource<byte>* element_source);
	};

    template <>
    struct __declspec(novtable) serialize<HeartbeatExtension>
    {
        void operator()(const HeartbeatExtension* value, IStream<byte>* stream) const
        {
            serialize<HeartbeatMode>()(&value->mode, stream);
        }
    };
}