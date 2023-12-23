// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.ServerSocket.h"
#include "Basic.ListenSocket.h"

namespace Basic
{
    ServerSocket::ServerSocket(std::shared_ptr<ITransportEventHandler<byte> > event_handler, uint32 receive_buffer_size) :
        ConnectedSocket(event_handler, receive_buffer_size)
    {
    }

    void ServerSocket::CompleteAccept(std::shared_ptr<ByteString> bytes, uint32 count)
    {
        sockaddr* localAddress;
        int localLength;

        sockaddr* remoteAddress;
        int remoteLength;

        GetAcceptExSockaddrs(
            bytes->address(),
            bytes->size() - (addressLength * 2),
            addressLength,
            addressLength,
            &localAddress,
            &localLength,
            &remoteAddress,
            &remoteLength);

        InitializePeer((sockaddr_in*)&remoteAddress);

        std::shared_ptr<ITransportEventHandler<byte> > event_handler = this->event_handler.lock();
        if (event_handler.get() == 0)
        {
            Disconnect();
            return;
        }

        event_handler->transport_connected();

        if (count > 0)
        {
            bytes->resize(count);
            Received(bytes.get());
        }

        if (socket != INVALID_SOCKET)
            StartReceive(bytes);
    }
}