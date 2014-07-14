// Copyright © 2014 Brian Spanton

#include "stdafx.h"
#include "Ftp.Server.h"
#include "Http.Types.h"
#include "Ftp.Globals.h"
#include "Basic.Event.h"

namespace Ftp
{
    using namespace Basic;

    Server::Server(std::shared_ptr<IProcess> completion, ByteStringRef cookie) :
        accept_completion(completion),
        accept_cookie(cookie),
        command_frame(&this->command)
    {
    }

    void Server::start(ListenSocket* listen_socket)
    {
        std::shared_ptr<ServerSocket> server_socket = std::make_shared<ServerSocket>(this->shared_from_this());

        this->peer = server_socket;
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
            if (event->get_type() == Basic::EventType::ready_for_write_bytes_event)
            {
                Basic::globals->DebugWriter()->WriteLine("accepted");

                std::shared_ptr<IProcess> completion = this->accept_completion;
                this->accept_completion = 0;

                Http::AcceptCompleteEvent event;
                event.cookie = this->accept_cookie;
                this->accept_cookie = 0;

                if (completion.get() != 0)
                    produce_event(completion.get(), &event);

                Ftp::globals->greeting->write_to_stream(this->peer.get());

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
                    // $$$ nyi
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