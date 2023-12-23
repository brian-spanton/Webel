// Copyright © 2014 Brian Spanton

#pragma once

#include "Service.Endpoint.h"

namespace Service
{
    using namespace Basic;

    class FtpServerEndpoint : public Endpoint, public std::enable_shared_from_this<FtpServerEndpoint>
    {
    public:
        FtpServerEndpoint(ListenSocket::Face face, short port);

        virtual void SpawnListener();
    };
}