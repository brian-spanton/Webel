// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ListenSocket.h"
#include "Http.RequestFrame.h"
#include "Tls.ICertificate.h"
#include "Web.Server.h"

namespace Service
{
    using namespace Http;

    class HttpServer : public Web::Server
    {
    private:
        static void AdminRequest(Request* request, Response* response);
        static void EchoRequest(Request* request, Response* response);
        static void QuestionRequest(Request* request, Response* response);

    public:
        typedef Basic::Ref<HttpServer, IProcess> Ref;

        virtual void Process();
    };
}