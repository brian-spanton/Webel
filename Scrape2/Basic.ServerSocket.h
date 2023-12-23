// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ConnectedSocket.h"

namespace Basic
{
    class ListenSocket;

    class ServerSocket : public ConnectedSocket
    {
    private:
        friend class ListenSocket;

        void CompleteAccept(std::shared_ptr<ByteString> bytes, uint32 count);

    public:
        ServerSocket(std::shared_ptr<ITransportEventHandler<byte> > event_handler, uint32 receive_buffer_size);
    };
}