// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Web.Server.h"
#include "Basic.ServerSocket.h"
#include "Basic.CountStream.h"
#include "Web.Globals.h"
#include "Http.Globals.h"
#include "Http.ResponseHeadersFrame.h"
#include "Http.HeadersFrame.h"
#include "Http.ResponseHeadersFrame.h"

namespace Web
{
    using namespace Basic;

    Server::Server(std::shared_ptr<IProcess> completion, ByteStringRef cookie) :
        accept_completion(completion),
        accept_cookie(cookie)
    {
    }

    void Server::start(ListenSocket* listen_socket, std::shared_ptr<Tls::ICertificate> certificate)
    {
        std::shared_ptr<ServerSocket> server_socket;
        Web::globals->CreateServerSocket(certificate, this->shared_from_this(), &server_socket, &this->peer);

        listen_socket->StartAccept(server_socket);
    }

    void Server::switch_to_state(State state)
    {
        __super::switch_to_state(state);

        if (!this->in_progress())
            this->peer->write_eof();
    }

    void Server::consider_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::pending_connection_state:
            {
                if (event->get_type() != Basic::EventType::ready_for_write_bytes_event)
                {
                    HandleError("unexpected event");
                    throw Yield("unexpected event");
                }

                Basic::globals->DebugWriter()->WriteLine("accepted");

                std::shared_ptr<IProcess> completion = this->accept_completion;
                this->accept_completion = 0;

                AcceptCompleteEvent event;
                event.cookie = this->accept_cookie;
                this->accept_cookie = 0;

                if (completion.get() != 0)
                    produce_event(completion.get(), &event);

                switch_to_state(State::new_request_state);
            }
            break;

        case State::new_request_state:
            {
                this->request = std::make_shared<Request>();
                this->request->Initialize();

                this->request_frame = std::make_shared<RequestFrame>(this->request.get());

                switch_to_state(State::request_frame_pending_state);
            }
            break;

        case State::request_frame_pending_state:
            {
                delegate_event_change_state_on_fail(this->request_frame.get(), event, State::request_frame_failed);

                Basic::globals->DebugWriter()->write_literal("Request received: ");
                serialize<Request>()(this->request.get(),  &Basic::globals->DebugWriter()->decoder);
                Basic::globals->DebugWriter()->WriteLine();

                this->response = std::make_shared<Response>();
                this->response->Initialize();

                handle_event();

                this->response->protocol = Http::globals->HTTP_1_1;

                if (this->response->server_body.get() != 0)
                {
                    Basic::CountStream<byte> count;

                    this->response->server_body->write_to_stream(&count);

                    this->response->headers->set_base_10(Http::globals->header_content_length, count.count);
                }
                else
                {
                    this->response->headers->set_base_10(Http::globals->header_content_length, 0);
                }

                Http::serialize<Response>()(this->response.get(), this->peer.get());
                this->peer->Flush();

                switch_to_state(State::response_done_state);

                ByteString response_bytes;
                Http::serialize<Response>()(this->response.get(), &response_bytes);

                Basic::globals->DebugWriter()->write_literal("Response sent: ");
                Basic::globals->DebugWriter()->write_elements((const char*)response_bytes.address(), response_bytes.size());
                Basic::globals->DebugWriter()->WriteLine();
            }
            break;

        case State::response_done_state:
            {
                UnicodeStringRef connection;
                bool success = this->request->headers->get_string(Http::globals->header_connection, &connection);
                if (success)
                {
                    if (equals<UnicodeString, false>(connection.get(), Http::globals->keep_alive.get()))
                    {
                        switch_to_state(State::new_request_state);
                        return;
                    }
                }

                switch_to_state(State::done_state);
            }
            break;

        default:
            throw FatalError("Web::Server::handle_event unexpected state");
        }
    }
}