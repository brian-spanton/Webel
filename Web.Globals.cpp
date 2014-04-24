// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Web.Globals.h"
#include "Tls.RecordLayer.h"

namespace Web
{
    using namespace Basic;

    Globals* globals = 0;

    Globals::Globals()
    {
    }

    void Globals::Initialize()
    {
    }

    void Globals::CreateServerSocket(Basic::Ref<Tls::ICertificate> certificate, IProcess* protocol, ServerSocket::Ref* socket, Basic::Ref<IBufferedStream<byte> >* peer)
    {
        ServerSocket::Ref server_socket = New<ServerSocket>();
        server_socket->Initialize();

        if (certificate.item() != 0)
        {
            Tls::RecordLayer::Ref tls_frame = New<Tls::RecordLayer>();
            tls_frame->Initialize(server_socket, protocol, true, certificate);

            server_socket->InitializeProtocol(tls_frame);
            (*peer) = tls_frame;
        }
        else
        {
            server_socket->InitializeProtocol(protocol);
            (*peer) = server_socket;
        }

        (*socket) = server_socket;
    }

    void Globals::CreateClientSocket(bool secure, IProcess* protocol, ClientSocket::Ref* socket, Basic::Ref<IBufferedStream<byte> >* peer)
    {
        ClientSocket::Ref client_socket = New<ClientSocket>();
        client_socket->Initialize();

        if (secure)
        {
            Tls::RecordLayer::Ref tls_frame = New<Tls::RecordLayer>();
            tls_frame->Initialize(client_socket, protocol, false, 0);

            client_socket->InitializeProtocol(tls_frame);
            (*peer) = tls_frame;
        }
        else
        {
            client_socket->InitializeProtocol(protocol);
            (*peer) = client_socket;
        }

        (*socket) = client_socket;
    }
};