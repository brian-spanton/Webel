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
            Basic::globals->HandleError("host is empty", 0);
            return false;
        }

        ADDRINFOEXW hints = {0};
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        ADDRINFOEXW* results;

		std::wstring wide_string;
		wide_string.reserve(host->size());

		// $ flawed conversion from utf-32 to utf-16
		for (uint32 i = 0; i < host->size(); i++)
		{
			wide_string.push_back((wchar_t)host->at(i));
		}

        // $ this DNS lookup isn't async, but should be.  possibly implement from scratch?
        int error = GetAddrInfoExW(wide_string.c_str(), 0, NS_DNS, 0, &hints, &results, 0, 0, 0, 0);
        if (error != NO_ERROR)
        {
            Basic::globals->HandleError("ClientSocket::Resolve WSAIoctl", WSAGetLastError());
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
            throw FatalError("ClientSocket::StartConnect this->state != State::new_state");

        sockaddr_in localAddress;
        localAddress.sin_family = AF_INET;
        localAddress.sin_port = 0;
        localAddress.sin_addr.S_un.S_addr = INADDR_ANY;

        int error = bind(this->socket, reinterpret_cast<const sockaddr*>(&localAddress), sizeof(localAddress));
        if (error == SOCKET_ERROR)
            throw FatalError("ClientSocket::StartConnect bind", WSAGetLastError());

        this->state = State::bound_state;

        InitializePeer(&remoteAddress);

        std::shared_ptr<SocketJobContext> job_context = std::make_shared<SocketJobContext>(SocketJobContext::ready_for_send_type);
        std::shared_ptr<Job> job = Job::make(this->shared_from_this(), job_context);

        Basic::globals->QueueJob(job);
    }

    void ClientSocket::CompleteReadyForSend()
    {
        std::shared_ptr<IProcess> protocol = this->protocol.lock();
        if (protocol.get() != 0)
        {
            ReadyForWriteBytesEvent event;
            event.Initialize(&this->protocol_element_source);
            produce_event(protocol.get(), &event);
        }
    }

    void ClientSocket::write_elements(const byte* elements, uint32 count)
    {
        if (count == 0)
            throw FatalError("ClientSocket::write_elements count == 0");

        if (this->state == State::receiving_state)
        {
            __super::write_elements(elements, count);
        }
        else
        {
            if (this->sendBuffer.get() == 0)
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
                Basic::globals->HandleError("ConnectedSocket::CompleteSend", error);

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

                    if (this->sendBuffer.get() != 0)
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
                throw FatalError("ClientSocket::CompleteSend unexpected state");
            }
        }
    }
}