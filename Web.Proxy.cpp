// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Web.Proxy.h"
#include "Basic.ServerSocket.h"
#include "Web.Globals.h"
#include "Http.Globals.h"
#include "Http.HeadersFrame.h"
#include "Basic.ClientSocket.h"

namespace Web
{
    using namespace Basic;

    Proxy::Proxy(std::shared_ptr<IProcess> call_back, std::shared_ptr<void> context, std::shared_ptr<Uri> server_url) :
        server_url(server_url),
        call_back(call_back),
        context(context)
    {
    }

    void Proxy::start(ListenSocket* listen_socket, std::shared_ptr<Tls::ICertificate> certificate)
    {
        std::shared_ptr<ServerSocket> server_socket;
        Web::globals->CreateServerSocket(certificate, this->shared_from_this(), 0x400, &server_socket, &this->client_transport);

        listen_socket->start_accept(server_socket, true);
    }

    void Proxy::switch_to_state(State state)
    {
        __super::switch_to_state(state);

        if (!this->in_progress())
        {
            if (this->client_transport)
            {
                this->client_transport->write_eof();
                this->client_transport.reset();
            }

            if (this->server_transport)
            {
                this->server_transport->write_eof();
                this->server_transport.reset();
            }
        }
    }

    ProcessResult Proxy::process_event(IEvent* event)
    {
        Hold hold(this->lock);

        if (event->get_type() == Basic::EventType::element_stream_ending_event)
        {
            switch_to_state(State::done_state);
            return ProcessResult::process_result_ready;
        }

        switch (get_state())
        {
        case State::pending_client_connection_state:
            {
                if (event->get_type() != Basic::EventType::can_send_bytes_event)
                {
                    StateMachine::LogUnexpectedEvent("Web", "Proxy", "process_event", event);
                    return ProcessResult::process_result_blocked;
                }

                Basic::LogInfo("Web", "Proxy", "process_event", "accepted connection");

                std::shared_ptr<IProcess> call_back = this->call_back.lock();
                if (call_back)
                {
                    AcceptCompleteEvent event;
                    event.context = this->context;
                    process_event_ignore_failures(call_back.get(), &event);
                }

                this->buffer = std::make_shared<ByteString>();

                std::shared_ptr<ServerProxy> server_proxy = std::make_shared<ServerProxy>();
                server_proxy->Initialize(this->shared_from_this());

                std::shared_ptr<ClientSocket> client_socket;
                Web::globals->CreateClientSocket(this->server_url->is_secure_scheme(), server_proxy, 0x400, &client_socket, &this->server_transport);

                sockaddr_in addr;
                bool success = client_socket->Resolve(this->server_url->host, this->server_url->get_port(), &addr);
                if (!success)
                {
                    Basic::LogDebug("Web", "Proxy", "process_event", "!client_socket->Resolve(this->server_url->host, ...)");
                    switch_to_state(State::done_state);
                    return ProcessResult::process_result_ready;
                }

                client_socket->StartConnect(addr);

                switch_to_state(State::pending_server_connection_state);
            }
            break;

        case State::pending_server_connection_state:
            {
                if (event->get_type() != Basic::EventType::received_bytes_event)
                {
                    StateMachine::LogUnexpectedEvent("Web", "Proxy", "process_event ", event);
                    throw FatalError();
                }

                ReceivedBytesEvent* read_event = (ReceivedBytesEvent*)event;

                const byte* bytes;
                uint32 count;

                read_event->element_source->Read(0xffffffff, &bytes, &count);

                if (count > 0)
                    this->buffer->insert(this->buffer->end(), bytes, bytes + count);
            }
            break;

        case State::connected_state:
            {
                if (event->get_type() != Basic::EventType::received_bytes_event)
                {
                    StateMachine::LogUnexpectedEvent("Web", "Proxy", "process_event", event);
                    throw FatalError();
                }

                ReceivedBytesEvent* read_event = (ReceivedBytesEvent*)event;

                const byte* bytes;
                uint32 count;

                read_event->element_source->Read(0xffffffff, &bytes, &count);

                if (count > 0)
                {
                    this->server_transport->write_elements(bytes, count);
                }
            }
            break;

        default:
            throw FatalError("Web", "Proxy", "process_event", "unhandled state", this->get_state());
        }

        return ProcessResult::process_result_ready;
    }

    ProcessResult Proxy::consider_server_event(IEvent* event)
    {
        Hold hold(this->lock);

        if (event->get_type() == Basic::EventType::element_stream_ending_event)
        {
            switch_to_state(State::done_state);
            return ProcessResult::process_result_ready;
        }

        switch (get_state())
        {
        case State::pending_server_connection_state:
            {
                if (event->get_type() != Basic::EventType::can_send_bytes_event)
                {
                    StateMachine::LogUnexpectedEvent("Web", "Proxy", "process_event", event);
                    return ProcessResult::process_result_blocked;
                }

                if (this->buffer->size() > 0)
                {
                    this->buffer->write_to_stream(this->server_transport.get());
                    this->buffer->clear();
                }

                switch_to_state(State::connected_state);
            }
            break;

        case State::connected_state:
            {
                if (event->get_type() != Basic::EventType::received_bytes_event)
                {
                    StateMachine::LogUnexpectedEvent("Web", "Proxy", "process_event", event);
                    throw FatalError();
                }

                ReceivedBytesEvent* read_event = (ReceivedBytesEvent*)event;

                const byte* bytes;
                uint32 count;

                read_event->element_source->Read(0xffffffff, &bytes, &count);

                if (count > 0)
                {
                    this->client_transport->write_elements(bytes, count);
                }
            }
            break;

        default:
            throw FatalError("Web", "Proxy", "process_event", "unhandled state", this->get_state());
        }

        return ProcessResult::process_result_ready;
    }

    void ServerProxy::Initialize(std::shared_ptr<Proxy> proxy)
    {
        this->proxy = proxy;
    }

    ProcessResult ServerProxy::process_event(IEvent* event)
    {
        return this->proxy->consider_server_event(event);
    }
}