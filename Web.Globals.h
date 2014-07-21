// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ClientSocket.h"
#include "Basic.ServerSocket.h"
#include "Tls.ICertificate.h"

namespace Web
{
    using namespace Basic;

    class Globals
    {
    public:
        Globals();

        void Initialize();

        void CreateServerSocket(std::shared_ptr<Tls::ICertificate> certificate, std::shared_ptr<IProcess> protocol, std::shared_ptr<Basic::ServerSocket>* socket, std::shared_ptr<IStream<byte> >* transport);
        void CreateClientSocket(bool secure, std::shared_ptr<IProcess> protocol, std::shared_ptr<Basic::ClientSocket>* socket, std::shared_ptr<IStream<byte> >* transport);
    };

    extern Globals* globals;
}
