// Copyright © 2014 Brian Spanton

#pragma once

#include "Service.Endpoint.h"
#include "Tls.ICertificate.h"

namespace Service
{
    using namespace Basic;

    class WebServerEndpoint : public Endpoint, public std::enable_shared_from_this<WebServerEndpoint>
    {
    private:
        std::shared_ptr<Tls::ICertificate> certificate;

    public:
        WebServerEndpoint(ListenSocket::Face face, short port, std::shared_ptr<Tls::ICertificate> certificate);

        virtual void SpawnListener();
    };
}