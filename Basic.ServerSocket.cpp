// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.ServerSocket.h"
#include "Basic.Event.h"
#include "Basic.ListenSocket.h"

namespace Basic
{
    ServerSocket::ServerSocket(std::shared_ptr<IProcess> protocol, uint32 receive_buffer_size) :
        ConnectedSocket(protocol, receive_buffer_size)
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

        char client_id[0x100];

        sprintf_s(
            client_id,
            "%d.%d.%d.%d:%d",
            this->remoteAddress.sin_addr.S_un.S_un_b.s_b1,
            this->remoteAddress.sin_addr.S_un.S_un_b.s_b2,
            this->remoteAddress.sin_addr.S_un.S_un_b.s_b3,
            this->remoteAddress.sin_addr.S_un.S_un_b.s_b4,
            this->remoteAddress.sin_port);

        LogEntry::current_call_context.clear();
        LogCallContextFrame call_frame(client_id);

        std::shared_ptr<IProcess> protocol = this->protocol.lock();
        if (!protocol)
        {
            Disconnect(0);
            return;
        }

        CanSendBytesEvent event;
        event.Initialize(&this->protocol_element_source);
        process_event_ignore_failures(protocol.get(), &event);

        if (count > 0)
        {
            bytes->resize(count);
            Received(bytes.get());
        }

        if (socket != INVALID_SOCKET)
            StartReceive(bytes);
    }
}