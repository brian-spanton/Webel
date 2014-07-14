// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Web.Proxy.h"
#include "Basic.ServerSocket.h"
#include "Web.Globals.h"
#include "Http.Globals.h"
#include "Http.ResponseHeadersFrame.h"
#include "Http.HeadersFrame.h"
#include "Basic.ClientSocket.h"

namespace Web
{
    using namespace Basic;

    Proxy::Proxy(std::shared_ptr<IProcess> completion, ByteStringRef cookie, std::shared_ptr<Uri> server_url) :
        server_url(server_url),
        accept_completion(completion),
        accept_cookie(cookie)
    {
    }

    void Proxy::start(ListenSocket* listen_socket, std::shared_ptr<Tls::ICertificate> certificate)
    {
        std::shared_ptr<ServerSocket> server_socket;
        Web::globals->CreateServerSocket(certificate, this->shared_from_this(), &server_socket, &this->client_peer);

        listen_socket->StartAccept(server_socket);
    }

    void Proxy::switch_to_state(State state)
    {
        __super::switch_to_state(state);

        if (!this->in_progress())
        {
            if (this->client_peer.get() != 0)
                this->client_peer->write_eof();

            if (this->server_peer.get() != 0)
                this->server_peer->write_eof();
        }
    }

    void Proxy::consider_event(IEvent* event)
    {
        Hold hold(this->lock);

        if (event->get_type() == Basic::EventType::element_stream_ending_event)
        {
            switch_to_state(State::done_state);
            return;
        }

        switch (get_state())
        {
        case State::pending_client_connection_state:
            {
                if (event->get_type() != Basic::EventType::ready_for_write_bytes_event)
                    throw FatalError("unexpected event");

                Basic::globals->DebugWriter()->WriteLine("accepted");

                std::shared_ptr<IProcess> completion = this->accept_completion;
                this->accept_completion = 0;

                AcceptCompleteEvent event;
                event.cookie = this->accept_cookie;
                this->accept_cookie = 0;

                if (completion.get() != 0)
                    produce_event(completion.get(), &event);

                this->buffer = std::make_shared<ByteString>();

                std::shared_ptr<ServerProxy> server_proxy = std::make_shared<ServerProxy>();
                server_proxy->Initialize(this->shared_from_this());

                std::shared_ptr<ClientSocket> client_socket;
                Web::globals->CreateClientSocket(this->server_url->is_secure_scheme(), server_proxy, &client_socket, &this->server_peer);

                sockaddr_in addr;
                bool success = client_socket->Resolve(this->server_url->host, this->server_url->get_port(), &addr);
                if (!success)
                {
                    HandleError("resolve failed");
                    switch_to_state(State::done_state);
                    return;
                }

                client_socket->StartConnect(addr);

                switch_to_state(State::pending_server_connection_state);
            }
            break;

        case State::pending_server_connection_state:
            {
                if (event->get_type() != Basic::EventType::ready_for_read_bytes_event)
                    throw FatalError("unexpected event");

                ReadyForReadBytesEvent* read_event = (ReadyForReadBytesEvent*)event;

                const byte* bytes;
                uint32 count;

                read_event->element_source->Read(0xffffffff, &bytes, &count);

                if (count > 0)
                    this->buffer->insert(this->buffer->end(), bytes, bytes + count);
            }
            break;

        case State::connected_state:
            {
                if (event->get_type() != Basic::EventType::ready_for_read_bytes_event)
                    throw FatalError("unexpected event");

                ReadyForReadBytesEvent* read_event = (ReadyForReadBytesEvent*)event;

                const byte* bytes;
                uint32 count;

                read_event->element_source->Read(0xffffffff, &bytes, &count);

                if (count > 0)
                {
                    this->server_peer->write_elements(bytes, count);
                    this->server_peer->Flush();
                }
            }
            break;

        default:
            throw FatalError("Web::Proxy::handle_event unexpected state");
        }
    }

    void Proxy::consider_server_event(IEvent* event)
    {
        Hold hold(this->lock);

        if (event->get_type() == Basic::EventType::element_stream_ending_event)
        {
            switch_to_state(State::done_state);
            return;
        }

        switch (get_state())
        {
        case State::pending_server_connection_state:
            {
                if (event->get_type() != Basic::EventType::ready_for_write_bytes_event)
                    throw FatalError("unexpected event");

                if (this->buffer->size() > 0)
                {
                    this->buffer->write_to_stream(this->server_peer.get());
                    this->buffer->clear();
                    this->server_peer->Flush();
                }

                switch_to_state(State::connected_state);
            }
            break;

        case State::connected_state:
            {
                if (event->get_type() != Basic::EventType::ready_for_read_bytes_event)
                    throw FatalError("unexpected event");

                ReadyForReadBytesEvent* read_event = (ReadyForReadBytesEvent*)event;

                const byte* bytes;
                uint32 count;

                read_event->element_source->Read(0xffffffff, &bytes, &count);

                if (count > 0)
                {
                    this->client_peer->write_elements(bytes, count);
                    this->client_peer->Flush();
                }
            }
            break;

        default:
            throw FatalError("Web::Proxy::handle_event unexpected state");
        }
    }

    void ServerProxy::Initialize(std::shared_ptr<Proxy> proxy)
    {
        this->proxy = proxy;
    }

    void ServerProxy::consider_event(IEvent* event)
    {
        this->proxy->consider_server_event(event);
    }
}