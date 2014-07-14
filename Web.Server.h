// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ListenSocket.h"
#include "Http.RequestFrame.h"
#include "Tls.ICertificate.h"

namespace Web
{
    using namespace Basic;
    using namespace Http;

    class Server : public Frame, public std::enable_shared_from_this<Server>
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

        std::shared_ptr<IBufferedStream<byte> > peer;
        std::shared_ptr<IProcess> accept_completion;
        ByteStringRef accept_cookie;
        std::shared_ptr<RequestFrame> request_frame;

        void switch_to_state(State state);

    protected:
        std::shared_ptr<Request> request;
        std::shared_ptr<Response> response;

        virtual void IProcess::consider_event(IEvent* event);

    public:
        Server(std::shared_ptr<IProcess> completion, ByteStringRef cookie);

        void start(ListenSocket* listen_socket, std::shared_ptr<Tls::ICertificate> certificate);
        virtual void handle_event() = 0;
    };
}