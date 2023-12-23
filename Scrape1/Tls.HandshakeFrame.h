// Copyright � 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Tls.SecurityParameters.h"
#include "Tls.Types.h"

namespace Tls
{
    using namespace Basic;

    class Server;

    class HandshakeFrame : public Frame
    {
    private:
        enum State
        {
            type_frame_pending_state = Start_State,
            length_frame_pending_state,
            done_state = Succeeded_State,
            type_frame_failed,
            length_frame_failed,
        };

        Handshake* handshake;
        NumberFrame<HandshakeType> type_frame;
        NumberFrame<uint32, 3> length_frame;

        virtual void IProcess::consider_event(IEvent* event);

    public:
        HandshakeFrame(Handshake* handshake);

        void reset();
    };

    template <>
    struct __declspec(novtable) serialize<Handshake>
    {
        void operator()(const Handshake* value, IStream<byte>* stream) const
        {
            serialize<HandshakeType>()(&value->msg_type, stream);
            serialize_number<uint32, 3>()(&value->length, stream);
        }
    };
}