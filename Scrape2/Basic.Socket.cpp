// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Socket.h"
#include "Basic.Globals.h"
#include "Basic.LogFrame.h"

namespace Basic
{
    SocketJobContext::SocketJobContext(Type type)
    {
        this->type = type;
    }

    Socket::Socket() :
        socket(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))
    {
        if (this->socket == INVALID_SOCKET)
            throw Basic::FatalError("socket", WSAGetLastError());

        Basic::globals->BindToCompletionQueue(this);
    }

    Socket::~Socket()
    {
        if (socket != INVALID_SOCKET)
            closesocket(socket);
    }

    void Socket::set_session_log(std::shared_ptr<ILog> session_log)
    {
        this->session_log = session_log;
    }

    void Socket::job_completed(std::shared_ptr<void> context, uint32 count, uint32 error)
    {
        Hold hold(this->lock);
        LogFrame log(this->session_log.lock());

        std::shared_ptr<SocketJobContext> socket_context = std::static_pointer_cast<SocketJobContext>(context);

        switch (socket_context->type)
        {
        case SocketJobContext::connect_type:
            CompleteConnect();
            break;

        case SocketJobContext::send_type:
            CompleteSend(socket_context->bytes, count, error);
            break;

        case SocketJobContext::receive_type:
            socket_context->bytes->resize(count);
            CompleteReceive(socket_context->bytes, error);
            break;

        case SocketJobContext::accept_type:
            CompleteAccept(socket_context->server_socket.get(), socket_context->bytes, count, error);
            break;

        case SocketJobContext::disconnect_type:
            CompleteDisconnect();
            break;
        }
    }

    void Socket::CompleteReceive(std::shared_ptr<ByteString> bytes, uint32 error)
    {
    }

    void Socket::CompleteSend(std::shared_ptr<ByteString> bytes, uint32 count, uint32 error)
    {
    }

    void Socket::CompleteConnect()
    {
    }

    void Socket::CompleteAccept(ServerSocket* server_socket, std::shared_ptr<ByteString> bytes, uint32 count, uint32 error)
    {
    }

    void Socket::CompleteDisconnect()
    {
    }
}