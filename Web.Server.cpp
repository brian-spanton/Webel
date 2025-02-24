// Copyright � 2013 Brian Spanton

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

    Server::Server(std::shared_ptr<IProcess> call_back, std::shared_ptr<void> context) :
        call_back(call_back),
        context(context)
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

    ProcessResult Server::process_event(IEvent* event)
    {
        if (event->get_type() == Basic::EventType::element_stream_ending_event)
        {
            switch_to_state(State::done_state);
            return ProcessResult::process_result_blocked;
        }

        switch (get_state())
        {
        case State::pending_connection_state:
            {
                if (event->get_type() != Basic::EventType::can_send_bytes_event)
                {
                    StateMachine::LogUnexpectedEvent("Web", "Server", "process_event", event);
                    return ProcessResult::process_result_blocked;
                }

                Basic::LogInfo("Web", "Server", "process_event", "accepted connection");

                std::shared_ptr<IProcess> call_back = this->call_back.lock();
                if (call_back)
                {
                    AcceptCompleteEvent event;
                    event.context = this->context;
                    process_event_ignore_failures(call_back.get(), &event);
                }

                switch_to_state(State::new_request_state);
                return ProcessResult::process_result_blocked;
            }
            break;

        case State::new_request_state:
            {
                this->request = std::make_shared<Request>();
                this->request->Initialize();

                this->request_frame = std::make_shared<RequestFrame>(this->request);

                switch_to_state(State::request_frame_pending_state);
            }
            break;

        case State::request_frame_pending_state:
            {
                ProcessResult result = process_event_change_state_on_fail(this->request_frame.get(), event, State::request_frame_failed);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                auto time_stamp = GetTickCount64();
                char request_id[0x100];
                int result2 = sprintf_s(request_id, "%llu", time_stamp);
                if (result2 == -1)
                    throw FatalError("Web", "Server", "process_event", "sprintf_s", result2);

                LogCallContextFrame call_frame(request_id);

			    std::shared_ptr<LogEntry> entry = std::make_shared<LogEntry>(LogLevel::Debug, "Web", "Server", "process_event");
                TextWriter(&entry->unicode_message).write_literal("Request received: ");
                this->request->render_request_line(&entry->unicode_message);
			    Basic::globals->add_entry(entry);

                this->response = std::make_shared<Response>();
                this->response->Initialize();

                handle_event();

                this->response->protocol = Http::globals->HTTP_1_1;

                if (this->response->response_body)
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

			    entry = std::make_shared<LogEntry>(LogLevel::Debug, "Web", "Server", "process_event");
                TextWriter(&entry->unicode_message).write_literal("Response sent: ");
                this->response->render_response_line(&entry->unicode_message);
			    Basic::globals->add_entry(entry);
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
            throw FatalError("Web", "Server", "process_event", "unhandled state", this->get_state());
        }

        return ProcessResult::process_result_ready;
    }
}