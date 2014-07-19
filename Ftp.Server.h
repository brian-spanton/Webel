// Copyright © 2014 Brian Spanton

#pragma once

#include "Basic.ListenSocket.h"
#include "Basic.Frame.h"
#include "Basic.CommandFrame.h"

namespace Ftp
{
    using namespace Basic;

    class Server : public Frame, public std::enable_shared_from_this<Server>
    {
    private:
        enum State
        {
            pending_connection_state = Start_State,
            command_frame_start_state,
            command_frame_pending_state,
            done_state = Succeeded_State,
        };

        std::shared_ptr<IBufferedStream<byte> > peer;
        std::weak_ptr<IProcess> completion;
        ByteStringRef completion_cookie;
        std::vector<Basic::ByteStringRef> command;
        Basic::CommandFrame<byte> command_frame;

        void switch_to_state(State state);

        virtual void IProcess::consider_event(IEvent* event);

    public:
        Server(std::shared_ptr<IProcess> completion, ByteStringRef cookie);

        void start(ListenSocket* listen_socket);
    };
}