// Copyright © 2014 Brian Spanton

#pragma once

#include "Service.Endpoint.h"
#include "Tls.ICertificate.h"
#include "Basic.Uri.h"

namespace Service
{
    using namespace Basic;

    class WebProxyEndpoint : public Endpoint, public std::enable_shared_from_this<WebProxyEndpoint>
    {
    private:
        std::shared_ptr<Tls::ICertificate> certificate;
        std::shared_ptr<Uri> server_url;

    public:
        WebProxyEndpoint(ListenSocket::Face face, short port, std::shared_ptr<Tls::ICertificate> certificate, std::shared_ptr<Uri> server_url);

        virtual void SpawnListener();
    };
}