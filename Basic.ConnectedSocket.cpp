// Copyright © 2013 Brian Spanton

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

        UnicodeStringRef id = std::make_shared<UnicodeString>();
        id->reserve(0x40);

        TextWriter text(id.get());
        text.WriteFormat<0x40>(
            "%d.%d.%d.%d:%d",
            this->remoteAddress.sin_addr.S_un.S_un_b.s_b1,
            this->remoteAddress.sin_addr.S_un.S_un_b.s_b2,
            this->remoteAddress.sin_addr.S_un.S_un_b.s_b3,
            this->remoteAddress.sin_addr.S_un.S_un_b.s_b4,
            this->remoteAddress.sin_port);

        // $ find a way to correlate this id to all related log entries
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
                if (protocol.get() == 0)
                {
                    Disconnect(0);
                    return;
                }

                this->protocol_element_source.Initialize(bytes->address(), bytes->size());

                ReadyForReadBytesEvent event;
                event.Initialize(&this->protocol_element_source);
                produce_event(protocol.get(), &event);

                if (!this->protocol_element_source.Exhausted() && !protocol->failed())
                    throw FatalError("Basic::ReadyForReadBytesEvent::Consume this->elements_read < this->count");
            }
        }
    }

    void ConnectedSocket::StartReceive(std::shared_ptr<ByteString> bytes)
    {
        uint32 count;
        DWORD flags = 0;

        std::shared_ptr<SocketJobContext> job_context = std::make_shared<SocketJobContext>(SocketJobContext::receive_type);
        job_context->bytes = bytes.get() != 0 ? bytes : std::make_shared<ByteString>();
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

        if (protocol.get() != 0)
        {
            ElementStreamEndingEvent event;
            produce_event(protocol.get(), &event);
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

    void ConnectedSocket::Send(std::shared_ptr<ByteString> bytes)
    {
        if (bytes->size() == 0)
            throw FatalError("ConnectedSocket::Send 0 bytes");

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
                Basic::globals->HandleError("ConnectedSocket::CompleteReceive", error);

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