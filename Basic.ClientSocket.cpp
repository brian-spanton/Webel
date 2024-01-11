// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.ClientSocket.h"
#include "Basic.Globals.h"
#include "Basic.Event.h"

namespace Basic
{
    ClientSocket::ClientSocket(std::shared_ptr<IProcess> protocol, uint32 receive_buffer_size) :
        ConnectedSocket(protocol, receive_buffer_size),
        state(State::new_state)
    {
    }

    bool ClientSocket::Resolve(UnicodeStringRef host, uint16 port, sockaddr_in* remoteAddress)
    {
        if (is_null_or_empty(host.get()))
        {
            Basic::LogError("Basic", "ClientSocket::Resolve { is_null_or_empty(host.get()) }");
            return false;
        }

        ADDRINFOEXA hints = {0};
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        ADDRINFOEXA* results;

        ByteString ascii_host;
        ascii_encode(host.get(), &ascii_host);

        // $ this DNS lookup isn't async, but should be.  possibly implement from scratch?
        int error = GetAddrInfoExA((char*)ascii_host.c_str(), 0, NS_DNS, 0, &hints, &results, 0, 0, 0, 0);
        if (error != NO_ERROR)
        {
            Basic::LogDebug("Basic", "ClientSocket::Resolve GetAddrInfoExA failed", WSAGetLastError());
            return false;
        }

        (*remoteAddress) = *reinterpret_cast<sockaddr_in*>(results->ai_addr);
        remoteAddress->sin_port = htons(port);
        return true;
    }

    void ClientSocket::StartConnect(IN_ADDR server, uint16 port)
    {
        sockaddr_in remoteAddress;
        remoteAddress.sin_family = AF_INET;
        remoteAddress.sin_addr = server;
        remoteAddress.sin_port = htons(port);

        StartConnect(remoteAddress);
    }

    void ClientSocket::StartConnect(sockaddr_in remoteAddress)
    {
        if (this->state != State::new_state)
            throw FatalError("Basic", "ClientSocket::StartConnect { this->state != State::new_state }");

        sockaddr_in localAddress;
        localAddress.sin_family = AF_INET;
        localAddress.sin_port = 0;
        localAddress.sin_addr.S_un.S_addr = INADDR_ANY;

        int error = bind(this->socket, reinterpret_cast<const sockaddr*>(&localAddress), sizeof(localAddress));
        if (error == SOCKET_ERROR)
            throw FatalError("Basic", "ClientSocket::StartConnect bind failed", WSAGetLastError());

        this->state = State::bound_state;

        InitializePeer(&remoteAddress);

        std::shared_ptr<SocketJobContext> job_context = std::make_shared<SocketJobContext>(SocketJobContext::can_send_bytes_event);
        std::shared_ptr<Job> job = Job::make(this->shared_from_this(), job_context);

        Basic::globals->QueueJob(job);
    }

    void ClientSocket::CompleteConnectionAccepted()
    {
        std::shared_ptr<IProcess> protocol = this->protocol.lock();
        if (protocol)
        {
            CanSendBytesEvent event;
            event.Initialize(&this->protocol_element_source);
            process_event_ignore_failures(protocol.get(), &event);
        }
    }

    void ClientSocket::write_elements(const byte* elements, uint32 count)
    {
        if (count == 0)
            throw FatalError("Basic", "ClientSocket::write_elements { count == 0 }");

        if (this->state == State::receiving_state)
        {
            __super::write_elements(elements, count);
        }
        else
        {
            if (!this->sendBuffer)
            {
                this->sendBuffer = std::make_shared<ByteString>();
                this->sendBuffer->reserve(0x1000);
            }

            this->sendBuffer->write_elements(elements, count);

            if (this->state == State::bound_state)
            {
                this->state = State::connecting_state;

                std::shared_ptr<ByteString> sendBuffer;
                sendBuffer.swap(this->sendBuffer);

                std::shared_ptr<SocketJobContext> job_context = std::make_shared<SocketJobContext>(SocketJobContext::send_type);
                job_context->bytes = sendBuffer;
                job_context->wsabuf.buf = (char*)job_context->bytes->address();
                job_context->wsabuf.len = job_context->bytes->size();

                std::shared_ptr<Job> job = Job::make(this->shared_from_this(), job_context);

                uint32 count;
                BOOL success = Basic::globals->ConnectEx(
                    this->socket,
                    reinterpret_cast<const sockaddr*>(&remoteAddress),
                    sizeof(remoteAddress),
                    (void*)sendBuffer->address(),
                    sendBuffer->size(),
                    &count,
                    job.get());
                if (success == FALSE)
                {
                    int error = WSAGetLastError();
                    if (error != ERROR_IO_PENDING)
                    {
                        job->Internal = error;
                        Basic::globals->QueueJob(job);
                    }
                }
            }
        }
    }

    void ClientSocket::CompleteSend(std::shared_ptr<ByteString> bytes, uint32 count, uint32 error)
    {
        if (error != ERROR_SUCCESS && error != STATUS_PENDING)
        {
            if (error != STATUS_CONNECTION_RESET && error != STATUS_CONNECTION_ABORTED)
                Basic::LogError("Basic", "ConnectedSocket::CompleteSend { error != STATUS_CONNECTION_RESET && error != STATUS_CONNECTION_ABORTED }", error);

            DisconnectAndNotifyProtocol();
        }
        else
        {
            switch (this->state)
            {
            case State::connecting_state:
                {
                    this->state = State::receiving_state;

                    StartReceive(bytes);

                    if (this->sendBuffer)
                    {
                        std::shared_ptr<ByteString> sendBuffer;
                        sendBuffer.swap(this->sendBuffer);

                        Send(sendBuffer);
                    }
                }
                break;

            case State::receiving_state:
                break;

            default:
                throw FatalError("Basic", "ClientSocket::CompleteSend unhandled state");
            }
        }
    }
}