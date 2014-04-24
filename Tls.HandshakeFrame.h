// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IProcess.h"
#include "Tls.SecurityParameters.h"
#include "Tls.Types.h"
#include "Tls.NumberFrame.h"

namespace Tls
{
    using namespace Basic;

    class Server;

    class HandshakeFrame : public Frame, public ISerializable
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
        Inline<NumberFrame<HandshakeType> > type_frame;
        Inline<NumberFrame<uint32, 3> > length_frame;

    public:
        typedef Basic::Ref<HandshakeFrame, IProcess> Ref;

        void Initialize(Handshake* handshake);

        virtual void IProcess::Process(IEvent* event, bool* yield);

        virtual void ISerializable::SerializeTo(IStream<byte>* stream);
    };
}