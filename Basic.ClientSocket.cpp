// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.ClientSocket.h"
#include "Basic.AsyncBytes.h"
#include "Basic.Globals.h"
#include "Basic.Event.h"

namespace Basic
{
    ClientSocket::ClientSocket()
    {
        this->sendBuffer.SetHolder(this);
    }

    void ClientSocket::Initialize()
    {
        __super::Initialize();
        this->state = State::new_state;
    }

    bool ClientSocket::Resolve(UnicodeString::Ref host, uint16 port, sockaddr_in* remoteAddress)
    {
        if (host.is_null_or_empty())
        {
            Basic::globals->HandleError("host is empty", 0);
            return false;
        }

        ADDRINFOEXA hints = {0};
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        ADDRINFOEXA* results;

        ZeroMemory(&resolveAddress, sizeof(resolveAddress));

        Inline<ByteString> host_bytes;
        host->ascii_encode(&host_bytes);

        // $ this DNS lookup isn't async, but should be.  possibly implement from scratch?
        int error = GetAddrInfoExA((const char*)host_bytes.c_str(), 0, NS_DNS, 0, &hints, &results, 0, 0, 0, 0);
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
            throw new Exception("ClientSocket::StartConnect this->state != State::new_state");

        sockaddr_in localAddress;
        localAddress.sin_family = AF_INET;
        localAddress.sin_port = 0;
        localAddress.sin_addr.S_un.S_addr = INADDR_ANY;

        int error = bind(this->socket, reinterpret_cast<const sockaddr*>(&localAddress), sizeof(localAddress));
        if (error == SOCKET_ERROR)
            throw new Exception("ClientSocket::StartConnect bind", WSAGetLastError());

        this->state = State::bound_state;

        InitializePeer(&remoteAddress);

        Basic::globals->PostCompletion(this, 0);
    }

    void ClientSocket::CompleteOther(int transferred, int error)
    {
        if (this->protocol.item() != 0)
        {
            ReadyForWriteBytesEvent event;
            event.Initialize(&this->protocol_element_source);
            this->protocol->Process(&event);
        }
    }

    void ClientSocket::Write(const byte* elements, uint32 count)
    {
        if (count == 0)
            throw new Exception("ClientSocket::Write count == 0");

        if (this->state == State::receiving_state)
        {
            __super::Write(elements, count);
        }
        else
        {
            if (this->sendBuffer.item() == 0)
            {
                this->sendBuffer = New<AsyncBytes>("1");
                this->sendBuffer->Initialize(0x1000);
            }

            this->sendBuffer->Write(elements, count);

            if (this->state == State::bound_state)
            {
                this->state = State::connecting_state;

                AsyncBytes::Ref sendBuffer = this->sendBuffer;
                this->sendBuffer = 0;
                sendBuffer->PrepareForSend("ClientSocket::Write ConnectEx", this);

                uint32 count;
                BOOL success = Basic::globals->ConnectEx(
                    this->socket,
                    reinterpret_cast<const sockaddr*>(&remoteAddress),
                    sizeof(remoteAddress),
                    sendBuffer->bytes,
                    sendBuffer->count,
                    &count,
                    sendBuffer);
                if (success == FALSE)
                {
                    int error = WSAGetLastError();
                    if (error != ERROR_IO_PENDING)
                    {
                        sendBuffer->Internal = error;
                        Basic::globals->PostCompletion(this, sendBuffer);
                    }
                }
            }
        }
    }

    void ClientSocket::CompleteWrite(AsyncBytes* bytes, int transferred, int error)
    {
        if (error != ERROR_SUCCESS && error != STATUS_PENDING)
        {
            if (error != STATUS_CONNECTION_RESET && error != STATUS_CONNECTION_ABORTED)
                Basic::globals->HandleError("ConnectedSocket::CompleteWrite", error);

            DisconnectAndNotifyProtocol();
        }
        else
        {
            switch (this->state)
            {
            case State::connecting_state:
                {
                    this->state = State::receiving_state;

                    StartReceive();

                    if (this->sendBuffer.item() != 0)
                    {
                        AsyncBytes::Ref sendBuffer = this->sendBuffer;
                        this->sendBuffer = 0;

                        Send(sendBuffer);
                    }
                }
                break;

            case State::receiving_state:
                break;

            default:
                throw new Exception("ClientSocket::CompleteWrite unexpected state");
            }
        }
    }
}