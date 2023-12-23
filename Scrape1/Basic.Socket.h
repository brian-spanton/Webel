// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ICompleter.h"

namespace Basic
{
    class ServerSocket;

    struct SocketJobContext
    {
        enum Type
        {
            ready_for_send_type,
            send_type,
            receive_type,
            accept_type,
            disconnect_type,
        };

        Type type;
        std::shared_ptr<ByteString> bytes;
        WSABUF wsabuf;
        std::shared_ptr<ServerSocket> server_socket;

        SocketJobContext(Type type);
    };

    class Socket : public ICompleter, public std::enable_shared_from_this<Socket>
    {
    protected:
        static const DWORD addressLength = sizeof(sockaddr_in) + 16;

        Lock lock;

        virtual void CompleteReceive(std::shared_ptr<ByteString> bytes, uint32 error);
        virtual void CompleteSend(std::shared_ptr<ByteString> bytes, uint32 count, uint32 error);
        virtual void CompleteReadyForSend();
        virtual void CompleteAccept(ServerSocket* server_socket, std::shared_ptr<ByteString> bytes, uint32 count, uint32 error);
        virtual void CompleteDisconnect();

    public:
        SOCKET socket;

        Socket();
        virtual ~Socket();

        virtual void ICompleter::complete(std::shared_ptr<void> context, uint32 count, uint32 error);
    };
}