// Copyright © 2014 Brian Spanton

#include "stdafx.h"
#include "Service.FtpServerEndpoint.h"
#include "Ftp.Server.h"

namespace Service
{
    using namespace Basic;

    FtpServerEndpoint::FtpServerEndpoint(ListenSocket::Face face, short port) :
        Endpoint(face, port)
    {
    }

    void FtpServerEndpoint::SpawnListener()
    {
        std::shared_ptr<Ftp::Server> protocol = std::make_shared<Ftp::Server>(this->shared_from_this(), ByteStringRef());
        protocol->start(this->listener.get());
    }
}