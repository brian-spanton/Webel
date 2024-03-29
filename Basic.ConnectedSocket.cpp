// Copyright � 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.ConnectedSocket.h"
#include "Basic.Globals.h"
#include "Basic.Hold.h"

namespace Basic
{
    ConnectedSocket::ConnectedSocket(std::shared_ptr<IProcess> protocol, uint32 receive_buffer_size) :
        protocol(protocol),
        receive_buffer_size(receive_buffer_size)
    {
    }

    void ConnectedSocket::InitializePeer(sockaddr_in* remoteAddress)
    {
        this->remoteAddress = (*remoteAddress);
    }

    void ConnectedSocket::Received(ByteString* bytes)
    {
        if (socket != INVALID_SOCKET)
        {
            if (bytes->size() == 0)
            {
                DisconnectAndNotifyProtocol();
            }
            else
            {
                std::shared_ptr<IProcess> protocol = this->protocol.lock();
                if (!protocol)
                {
                    Disconnect(0);
                    return;
                }

                this->protocol_element_source.Initialize(bytes->address(), bytes->size());

                ReceivedBytesEvent event;
                event.Initialize(&this->protocol_element_source);
                process_event_ignore_failures(protocol.get(), &event);

                if (!this->protocol_element_source.Exhausted() && !protocol->failed())
                    throw FatalError("Basic", "ConnectedSocket", "Received", "!this->protocol_element_source.Exhausted() && !protocol->failed()");
            }
        }
    }

    void ConnectedSocket::StartReceive(std::shared_ptr<ByteString> bytes)
    {
        uint32 count;
        DWORD flags = 0;

        std::shared_ptr<SocketJobContext> job_context = std::make_shared<SocketJobContext>(SocketJobContext::receive_type);
        job_context->bytes = bytes ? bytes : std::make_shared<ByteString>();
        job_context->bytes->resize(0x400);
        job_context->wsabuf.buf = (char*)job_context->bytes->address();
        job_context->wsabuf.len = job_context->bytes->size();

        std::shared_ptr<Job> job = Job::make(this->shared_from_this(), job_context);

        int error = WSARecv(socket, &job_context->wsabuf, 1, &count, &flags, job.get(), 0);
        if (error == SOCKET_ERROR)
        {
            error = WSAGetLastError();
            if (error != ERROR_IO_PENDING)
            {
                job->Internal = error;
                Basic::globals->QueueJob(job);
            }
        }
    }

    void ConnectedSocket::DisconnectAndNotifyProtocol()
    {
        std::shared_ptr<IProcess> protocol;
        Disconnect(&protocol);

        if (protocol)
        {
            ElementStreamEndingEvent event;
            process_event_ignore_failures(protocol.get(), &event);
        }
    }

    void ConnectedSocket::Disconnect(std::shared_ptr<IProcess>* protocol)
    {
        if (socket != INVALID_SOCKET)
        {
            shutdown(socket, SD_BOTH);
            closesocket(socket);

            socket = INVALID_SOCKET;
        }

        if (protocol != 0)
            (*protocol) = this->protocol.lock();

        this->protocol.reset();
    }

    void ConnectedSocket::write_elements(const byte* elements, uint32 count)
    {
        while(true)
        {
            if (count == 0)
                break;

            std::shared_ptr<ByteString> buffer = std::make_shared<ByteString>();
            buffer->reserve(0x1000);

            uint32 remaining = 0x1000 - buffer->size();
            uint32 useable = (count > remaining) ? remaining : count;

            buffer->write_elements(elements, useable);

            count -= useable;
            elements += useable;

            Send(buffer);
        }
    }

    void ConnectedSocket::write_element(byte element)
    {
        write_elements(&element, 1);
    }

    void ConnectedSocket::Send(ByteStringRef bytes)
    {
        if (bytes->size() == 0)
            throw FatalError("Basic", "ConnectedSocket", "Send", "bytes->size() == 0");

        uint32 count;
        DWORD flags = 0;

        std::shared_ptr<SocketJobContext> job_context = std::make_shared<SocketJobContext>(SocketJobContext::send_type);
        job_context->bytes = bytes;
        job_context->wsabuf.buf = (char*)job_context->bytes->address();
        job_context->wsabuf.len = job_context->bytes->size();

        std::shared_ptr<Job> job = Job::make(this->shared_from_this(), job_context);

        int error = WSASend(socket, &job_context->wsabuf, 1, &count, flags, job.get(), 0);
        if (error == SOCKET_ERROR)
        {
            error = WSAGetLastError();
            if (error != ERROR_IO_PENDING)
            {
                job->Internal = error;
                Basic::globals->QueueJob(job);
            }
        }
    }

    void ConnectedSocket::write_eof()
    {
        std::shared_ptr<SocketJobContext> job_context = std::make_shared<SocketJobContext>(SocketJobContext::disconnect_type);
        std::shared_ptr<Job> job = Job::make(this->shared_from_this(), job_context);
        Basic::globals->QueueJob(job);
    }

    void ConnectedSocket::CompleteReceive(std::shared_ptr<ByteString> bytes, uint32 error)
    {
        if (error != ERROR_SUCCESS && error != STATUS_PENDING)
        {
            if (error != STATUS_CONNECTION_RESET && error != STATUS_CONNECTION_ABORTED && error != STATUS_CANCELLED)
                Basic::LogError("Basic", "ConnectedSocket", "CompleteReceive", "error != STATUS_CONNECTION_RESET && error != STATUS_CONNECTION_ABORTED && error != STATUS_CANCELLED", error);

            DisconnectAndNotifyProtocol();
        }
        else
        {
            Received(bytes.get());

            if (socket != INVALID_SOCKET)
                StartReceive(bytes);
        }
    }

    void ConnectedSocket::CompleteDisconnect()
    {
        Disconnect(0);
    }
}