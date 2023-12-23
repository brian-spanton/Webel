// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ListenSocket.h"
#include "Http.RequestFrame.h"
#include "Tls.ICertificate.h"

namespace Web
{
    using namespace Basic;
    using namespace Http;

    class Proxy : public Frame, public std::enable_shared_from_this<Proxy>
    {
    private:
        enum State
        {
            pending_client_connection_state = Start_State,
            pending_server_connection_state,
            connected_state,
            done_state = Succeeded_State,
        };

        std::shared_ptr<IStream<byte> > client_transport;
        std::shared_ptr<IStream<byte> > server_transport;
        std::weak_ptr<IProcess> completion;
        ByteStringRef completion_cookie;
        std::shared_ptr<Uri> server_url;
        Lock lock;
        std::shared_ptr<ByteString> buffer;

        void switch_to_state(State state);

        virtual void IProcess::consider_event(IEvent* event);

    public:
        Proxy(std::shared_ptr<IProcess> completion, ByteStringRef cookie, std::shared_ptr<Uri> server_url);

        void start(ListenSocket* listen_socket, std::shared_ptr<Tls::ICertificate> certificate);
        void consider_server_event(IEvent* event);
    };

    class ServerProxy : public Frame
    {
    private:
        std::shared_ptr<Proxy> proxy;

        virtual void IProcess::consider_event(IEvent* event);

    public:
        void Initialize(std::shared_ptr<Proxy> proxy);
    };
}