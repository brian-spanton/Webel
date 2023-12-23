// Copyright © 2014 Brian Spanton

#include "stdafx.h"
#include "Service.WebProxyEndpoint.h"
#include "Web.Proxy.h"

namespace Service
{
    using namespace Basic;

    WebProxyEndpoint::WebProxyEndpoint(ListenSocket::Face face, short port, std::shared_ptr<Tls::ICertificate> certificate, std::shared_ptr<Uri> server_url) :
        Endpoint(face, port),
        certificate(certificate),
        server_url(server_url)
    {
    }

    void WebProxyEndpoint::SpawnListener()
    {
        std::shared_ptr<Web::Proxy> protocol = std::make_shared<Web::Proxy>(this->shared_from_this(), ByteStringRef(), this->server_url);
        protocol->start(this->listener.get(), this->certificate);
    }
}