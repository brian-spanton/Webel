// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Tls.Types.h"

namespace Tls
{
    using namespace Basic;

    class HeartbeatMessageFrame : public Frame
    {
    private:
        enum State
        {
            type_frame_pending_state = Start_State,
            payload_length_frame_pending_state,
            payload_frame_pending_state,
            padding_frame_pending_state,
            done_state = Succeeded_State,
            type_frame_failed,
            payload_length_frame_failed,
            payload_length_error,
            payload_frame_failed,
            padding_frame_failed,
        };

        HeartbeatMessage* heartbeat_message = 0;
        uint32 plaintext_length = 0;
        uint32 padding_length = 0;
        NumberFrame<HeartbeatMessageType> type_frame;
        NumberFrame<uint16> payload_length_frame;
        MemoryRange payload_frame;
        MemoryRange padding_frame;

        virtual event_result IProcess::consider_event(IEvent* event);

    public:
        HeartbeatMessageFrame(HeartbeatMessage* heartbeat_message);
        
        void set_plaintext_length(uint32 plaintext_length);
    };

    template <>
    struct __declspec(novtable) serialize<HeartbeatMessage>
    {
        void operator()(const HeartbeatMessage* value, IStream<byte>* stream) const
        {
            serialize<HeartbeatMessageType>()(&value->type, stream);
            serialize<uint16>()(&value->payload_length, stream);
            value->payload.write_to_stream(stream);
            value->padding.write_to_stream(stream);
        }
    };
}