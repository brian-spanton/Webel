// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ServerSocket.h"
#include "Basic.Hold.h"

namespace Basic
{
    class ListenSocket : public Socket
    {
    protected:
        virtual void CompleteAccept(ServerSocket* server_socket, std::shared_ptr<ByteString> bytes, uint32 count, uint32 error);

    public:
        enum Face
        {
            Face_Local,
            Face_External,
            Face_Default,
        };

        ListenSocket(Face face, short port);

        void StartAccept(std::shared_ptr<ServerSocket> acceptPeer);
    };
}