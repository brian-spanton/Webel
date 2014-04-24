// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ListenSocket.h"
#include "Http.RequestFrame.h"
#include "Tls.ICertificate.h"

namespace Web
{
    using namespace Basic;
    using namespace Http;

    class Server : public Frame
    {
    private:
        enum State
        {
            pending_connection_state = Start_State,
            new_request_state,
            request_frame_pending_state,
            response_done_state,
            done_state = Succeeded_State,
            request_frame_failed,
        };

        Basic::Ref<IBufferedStream<byte> > peer; // REF
        Basic::Ref<IProcess> accept_completion; // REF
        ByteString::Ref accept_cookie; // REF
        Inline<RequestFrame> request_frame;

        void switch_to_state(State state);

    protected:
        Request::Ref request; // REF
        Response::Ref response; // REF

    public:
        typedef Basic::Ref<Server, IProcess> Ref;

        void Initialize(ListenSocket* listen_socket, Basic::Ref<Tls::ICertificate> certificate, Basic::Ref<IProcess> completion, ByteString::Ref cookie);

        virtual void IProcess::Process(IEvent* event, bool* yield);
        virtual void Process() = 0;
    };
}