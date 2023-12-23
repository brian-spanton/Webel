// Copyright � 2014 Brian Spanton

#include "stdafx.h"
#include "Ftp.Server.h"
#include "Http.Types.h"
#include "Ftp.Globals.h"

namespace Ftp
{
    using namespace Basic;

    Server::Server(std::shared_ptr<IServerCompletion> completion, ByteStringRef cookie) :
        completion(completion),
        completion_cookie(cookie),
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

    void Server::consider_event(void* event)
    {
        switch (get_state())
        {
        case State::pending_connection_state:
            if (event->get_type() == Basic::EventType::ready_for_write_bytes_event)
            {
                TextWriter(Basic::globals->LogStream()).write_line("accepted");

                std::shared_ptr<IProcess> completion = this->completion.lock();
                if (completion.get() != 0)
                {
                    Http::AcceptCompleteEvent event;
                    event.cookie = this->completion_cookie;
                    produce_event(completion.get(), &event);
                }

                Ftp::globals->greeting->write_to_stream(this->transport.get());

                switch_to_state(State::command_frame_start_state);
            }
            else
            {
                throw FatalError("unexpected event");
            }
            break;

        case State::command_frame_start_state:
            this->command.clear();
            this->command_frame.reset();
            switch_to_state(State::command_frame_pending_state);
            break;

        case State::command_frame_pending_state:
            {
                delegate_event(&this->command_frame, event);

                if (this->command_frame.failed())
                {
                    // $$ nyi
                }

                switch_to_state(State::command_frame_start_state);

                bool handled = false;

                if (!handled)
                {
                }
            }
            break;

        default:
            throw FatalError("Ftp::Server::handle_event unexpected state");
        }
    }
}