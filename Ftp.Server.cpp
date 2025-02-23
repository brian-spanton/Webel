// Copyright © 2014 Brian Spanton

#include "stdafx.h"
#include "Ftp.Server.h"
#include "Http.Types.h"
#include "Ftp.Globals.h"
#include "Basic.Event.h"

namespace Ftp
{
    using namespace Basic;

    Server::Server(std::shared_ptr<IProcess> call_back, std::shared_ptr<void> context) :
        call_back(call_back),
        context(context),
        command_frame(&this->command) // initialization is in order of declaration in class def
    {
    }

    void Server::start(ListenSocket* listen_socket)
    {
        std::shared_ptr<ServerSocket> server_socket = std::make_shared<ServerSocket>(this->shared_from_this(), 0x400);

        this->transport = server_socket;
        listen_socket->start_accept(server_socket, false);
    }

    void Server::switch_to_state(State state)
    {
        __super::switch_to_state(state);

        if (!this->in_progress())
        {
            this->transport->write_eof();
            this->transport.reset();
        }
    }

    ProcessResult Server::process_event(IEvent* event)
    {
        switch (get_state())
        {
        case State::pending_connection_state:
            if (event->get_type() == Basic::EventType::can_send_bytes_event)
            {
                Basic::LogInfo("Ftp", "Server", "process_event", "accepted connection");

                std::shared_ptr<IProcess> call_back = this->call_back.lock();
                if (call_back)
                {
                    Http::AcceptCompleteEvent event;
                    event.context = this->context;
                    process_event_ignore_failures(call_back.get(), &event);
                }

                Ftp::globals->greeting->write_to_stream(this->transport.get());

                switch_to_state(State::command_frame_start_state);
            }
            else
            {
                StateMachine::LogUnexpectedEvent("Ftp", "Server", "process_event", event);
                throw FatalError();
            }
            break;

        case State::command_frame_start_state:
            this->command.clear();
            this->command_frame.reset();
            switch_to_state(State::command_frame_pending_state);
            break;

        case State::command_frame_pending_state:
            {
                ProcessResult result = Basic::process_event(&this->command_frame, event);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                if (this->command_frame.failed())
                {
                    // $$ NYI
                }

                switch_to_state(State::command_frame_start_state);

                bool handled = false;

                if (!handled)
                {
                }
            }
            break;

        default:
            throw FatalError("Ftp", "Server", "process_event", "unhandled state", this->get_state());
        }

        return ProcessResult::process_result_ready;
    }
}