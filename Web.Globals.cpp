// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Web.Globals.h"
#include "Tls.RecordLayer.h"
#include "Tls.ServerNameFrame.h"
#include "Tls.SignatureAndHashAlgorithmFrame.h"

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

    void Globals::CreateServerSocket(std::shared_ptr<Tls::ICertificate> certificate, std::shared_ptr<IProcess> protocol, uint32 receive_buffer_size, std::shared_ptr<ServerSocket>* socket, std::shared_ptr<IStream<byte> >* transport)
    {
        if (certificate.get() != 0)
        {
            std::shared_ptr<Tls::RecordLayer> tls_frame = std::make_shared<Tls::RecordLayer>(protocol, true, certificate);
            std::shared_ptr<ServerSocket> server_socket = std::make_shared<ServerSocket>(tls_frame, receive_buffer_size);

            tls_frame->set_transport(server_socket);

            (*transport) = tls_frame;
            (*socket) = server_socket;
        }
        else
        {
            std::shared_ptr<ServerSocket> server_socket = std::make_shared<ServerSocket>(protocol, receive_buffer_size);

            (*transport) = server_socket;
            (*socket) = server_socket;
        }
    }

    void Globals::CreateClientSocket(bool secure, std::shared_ptr<IProcess> protocol, uint32 receive_buffer_size, std::shared_ptr<ClientSocket>* socket, std::shared_ptr<IStream<byte> >* transport)
    {
        if (secure)
        {
            std::shared_ptr<Tls::RecordLayer> tls_frame = std::make_shared<Tls::RecordLayer>(protocol, false, std::shared_ptr<Tls::ICertificate>());
            std::shared_ptr<ClientSocket> client_socket = std::make_shared<ClientSocket>(tls_frame, receive_buffer_size);

            tls_frame->set_transport(client_socket);

            (*transport) = tls_frame;
            (*socket) = client_socket;
        }
        else
        {
            std::shared_ptr<ClientSocket> client_socket = std::make_shared<ClientSocket>(protocol, receive_buffer_size);

            (*transport) = client_socket;
            (*socket) = client_socket;
        }
    }
};