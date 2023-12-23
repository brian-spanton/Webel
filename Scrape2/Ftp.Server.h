// Copyright © 2014 Brian Spanton

#pragma once

#include "Basic.StateMachine.h"
#include "Basic.ListenSocket.h"
#include "Basic.CommandBuilder.h"

namespace Ftp
{
    using namespace Basic;

    __interface IServerCompletion
    {
    };

    class Server : public StateMachine, public std::enable_shared_from_this<Server>
    {
    private:
        enum State
        {
            pending_connection_state = Start_State,
            command_frame_start_state,
            command_frame_pending_state,
            done_state = Succeeded_State,
        };

        std::shared_ptr<IStream<byte> > transport;
        std::weak_ptr<IServerCompletion> completion;
        ByteStringRef completion_cookie;
        std::vector<Basic::ByteStringRef> command;
        Basic::CommandBuilder<byte> command_frame;

        void switch_to_state(State state);

        void consider_event(void* event);

    public:
        Server(std::shared_ptr<IServerCompletion> completion, ByteStringRef cookie);

        void start(ListenSocket* listen_socket);
    };
}