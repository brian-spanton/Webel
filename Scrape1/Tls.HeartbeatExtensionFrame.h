// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Tls.Types.h"

namespace Tls
{
    using namespace Basic;

    class HeartbeatExtensionFrame : public Frame
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

        virtual void IProcess::consider_event(IEvent* event);

    public:
        HeartbeatExtensionFrame(HeartbeatExtension* heartbeat_extension);
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