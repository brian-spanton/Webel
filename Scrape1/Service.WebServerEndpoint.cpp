// Copyright © 2014 Brian Spanton

#include "stdafx.h"
#include "Service.WebServerEndpoint.h"
#include "Service.WebServer.h"

namespace Service
{
    using namespace Basic;

    WebServerEndpoint::WebServerEndpoint(ListenSocket::Face face, short port, std::shared_ptr<Tls::ICertificate> certificate) :
        Endpoint(face, port),
        certificate(certificate)
    {
    }

    void WebServerEndpoint::SpawnListener()
    {
        std::shared_ptr<WebServer> protocol = std::make_shared<WebServer>(this->shared_from_this(), ByteStringRef());
        protocol->start(this->listener.get(), this->certificate);
    }
}