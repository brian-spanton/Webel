// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Web.Server.h"
#include "Basic.ServerSocket.h"
#include "Basic.CountStream.h"
#include "Web.Globals.h"
#include "Http.Globals.h"
#include "Http.ResponseFrame.h"
#include "Http.HeadersFrame.h"

namespace Web
{
    using namespace Basic;

    Server::Server(std::shared_ptr<IProcess> completion, ByteStringRef cookie) :
        completion(completion),
        completion_cookie(cookie)
    {
    }

    void Server::start(ListenSocket* listen_socket, std::shared_ptr<Tls::ICertificate> certificate)
    {
        // keep ourself alive until we decide to self-destruct
        this->self = this->shared_from_this();

        std::shared_ptr<ServerSocket> server_socket;
        Web::globals->CreateServerSocket(certificate, this->self, 0x400, &server_socket, &this->transport);

        listen_socket->start_accept(server_socket, true);
    }

    void Server::close_transport()
    {
        this->transport->write_eof();
        this->transport.reset();
    }

    void Server::switch_to_state(State state)
    {
        __super::switch_to_state(state);

        if (!this->in_progress())
        {
            this->self.reset();
        }
    }

    ProcessResult Server::consider_event(IEvent* event)
    {
        if (event->get_type() == Basic::EventType::element_stream_ending_event)
        {
            switch_to_state(State::done_state);
            return ProcessResult::process_result_blocked; // event consumed
        }

        switch (get_state())
        {
        case State::pending_connection_state:
            {
                if (event->get_type() != Basic::EventType::can_send_bytes_event)
                {
                    HandleError("unexpected event");
                    return ProcessResult::process_result_blocked; // unexpected event
                }

                Basic::globals->DebugWriter()->WriteLine("accepted");

                std::shared_ptr<IProcess> completion = this->completion.lock();
                if (completion.get() != 0)
                {
                    AcceptCompleteEvent event;
                    event.cookie = this->completion_cookie;
                    produce_event(completion.get(), &event);
                }

                switch_to_state(State::new_request_state);
                return ProcessResult::process_result_blocked; // event consumed
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
                ProcessResult result = process_event_change_state_on_fail(this->request_frame.get(), event, State::request_frame_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                Basic::globals->DebugWriter()->write_literal("Request received: ");
                render_request_line(this->request.get(),  &Basic::globals->DebugWriter()->decoder);
                Basic::globals->DebugWriter()->WriteLine();

                this->response = std::make_shared<Response>();
                this->response->Initialize();

                handle_event();

                this->response->protocol = Http::globals->HTTP_1_1;

                if (this->response->response_body.get() != 0)
                {
                    Basic::CountStream<byte> count;

                    this->response->response_body->write_to_stream(&count);

                    this->response->headers->set_base_10(Http::globals->header_content_length, count.count);
                }
                else
                {
                    this->response->headers->set_base_10(Http::globals->header_content_length, 0);
                }

                ByteString response_bytes;
                Http::serialize<Response>()(this->response.get(), &response_bytes);
                response_bytes.write_to_stream(this->transport.get());

                switch_to_state(State::response_done_state);

                Basic::globals->DebugWriter()->write_literal("Response sent: ");
                this->response->render_response_line(&Basic::globals->DebugWriter()->decoder);
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
                        return ProcessResult::process_result_ready;
                    }
                }

                close_transport();
                switch_to_state(State::done_state);
            }
            break;

        default:
            throw FatalError("Web::Server::handle_event unexpected state");
        }

        return ProcessResult::process_result_ready;
    }
}