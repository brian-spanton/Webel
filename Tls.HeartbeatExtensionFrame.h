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

        virtual ProcessResult IProcess::consider_event(IEvent* event);

    public:
        HeartbeatExtensionFrame(HeartbeatExtension* heartbeat_extension);
    };
}

namespace Basic
{
    template <>
    struct __declspec(novtable) serialize<Tls::HeartbeatExtension>
    {
        void operator()(const Tls::HeartbeatExtension* value, IStream<byte>* stream) const
        {
            serialize<Tls::HeartbeatMode>()(&value->mode, stream);
        }
    };
}