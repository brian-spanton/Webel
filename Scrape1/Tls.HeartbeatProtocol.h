// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Tls.Types.h"
#include "Tls.HeartbeatMessageFrame.h"

namespace Tls
{
    using namespace Basic;

    class RecordLayer;

    class HeartbeatProtocol : public Frame
    {
    protected:
        enum State
        {
            start_state = Start_State,
            heartbeat_message_frame_pending_state,
            done_state = Succeeded_State,
            heartbeat_message_frame_failed,
            unexpected_type_error,
        };

        RecordLayer* session;
        HeartbeatMessage heartbeat_message;
        HeartbeatMessageFrame heartbeat_message_frame;

        virtual void IProcess::consider_event(IEvent* event);

    public:
        HeartbeatProtocol(RecordLayer* session);
        void SetPlaintextLength(uint16 plaintext_length);
    };
}