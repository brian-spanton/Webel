// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.ListenSocket.h"

namespace Basic
{
    ListenSocket::ListenSocket(Face face, short port)
    {
        sockaddr_in endpoint;
        endpoint.sin_family = AF_INET;
        endpoint.sin_port = htons(port);

        switch (face)
        {
        case Face_Local:
            endpoint.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
            break;

        case Face_External:
            {
                hostent* host = gethostbyname(0);
                endpoint.sin_addr = *reinterpret_cast<in_addr*>(host->h_addr_list[0]);
            }
            break;

        case Face_Default:
            endpoint.sin_addr.S_un.S_addr = INADDR_ANY;
            break;
        }

        int error = bind(socket, reinterpret_cast<const sockaddr*>(&endpoint), sizeof(endpoint));
        if (error == SOCKET_ERROR)
            throw FatalError("bind", WSAGetLastError());

        error = listen(socket, SOMAXCONN);
        if (error == SOCKET_ERROR)
            throw FatalError("listen", WSAGetLastError());

        Basic::globals->DebugWriter()->WriteFormat<0x40>("listening on port %d", port);
        Basic::globals->DebugWriter()->WriteLine();
    }

    void ListenSocket::start_accept(std::shared_ptr<ServerSocket> server_socket, bool receive_with_connect)
    {
        std::shared_ptr<SocketJobContext> job_context = std::make_shared<SocketJobContext>(SocketJobContext::accept_type);
        job_context->bytes = std::make_shared<ByteString>();

        // setting this guy to just room for the connection addresses ensures we don't
        // wait for data to be sent before completion of the accept.  It is slightly more
        // efficient to wait for data, but that presumes the protocol expects the client
        // to send upon connection.  not all protocols work this way (ftp for instance).
        uint32 receive_buffer_size = receive_with_connect ? server_socket->receive_buffer_size : 0;

        job_context->bytes->resize(receive_buffer_size + addressLength * 2);

        job_context->wsabuf.buf = (char*)job_context->bytes->address();
        job_context->wsabuf.len = job_context->bytes->size();
        job_context->server_socket = server_socket;

        std::shared_ptr<Job> job = Job::make(this->shared_from_this(), job_context);

        uint32 count;
        BOOL success2 = AcceptEx(
            this->socket,
            server_socket->socket,
            job_context->bytes->address(),
            job_context->bytes->size() - (addressLength * 2),
            addressLength,
            addressLength,
            &count,
            job.get());
        if (success2 == FALSE)
        {
            int error = WSAGetLastError();
            if (error != ERROR_IO_PENDING)
            {
                job->Internal = error;
                Basic::globals->QueueJob(job);
            }
        }
    }

    void ListenSocket::CompleteAccept(ServerSocket* server_socket, std::shared_ptr<ByteString> bytes, uint32 count, uint32 error)
    {
        if (error != ERROR_SUCCESS)
        {
            if (error != STATUS_CONNECTION_RESET && error != STATUS_CANCELLED)
                throw FatalError("ListenSocket::CompleteRead", error);
        }
        else
        {
            error = setsockopt(
                server_socket->socket,
                SOL_SOCKET,
                SO_UPDATE_ACCEPT_CONTEXT,
                reinterpret_cast<char*>(&this->socket),
                sizeof(this->socket));
            if (error == SOCKET_ERROR)
                throw FatalError("ListenSocket::CompleteRead setsockopt", WSAGetLastError());

            server_socket->CompleteAccept(bytes, count);
        }
    }
}