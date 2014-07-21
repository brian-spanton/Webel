// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ListenSocket.h"
#include "Http.RequestFrame.h"
#include "Tls.ICertificate.h"
#include "Web.Server.h"

namespace Service
{
    using namespace Http;

    class WebServer : public Web::Server
    {
    private:
        static void AdminRequest(Request* request, Response* response);
        static void EchoRequest(Request* request, Response* response);
        static void QuestionRequest(Request* request, Response* response);
        static void LogRequest(Request* request, Response* response);

    public:
        WebServer(std::shared_ptr<IProcess> completion, ByteStringRef cookie);

        virtual void handle_event();
    };
}