// Copyright © 2013 Brian Spanton

#pragma once

#include "Http.ResponseHeadersFrame.h"
#include "Http.BodyFrame.h"
#include "Http.MediaType.h"
#include "Basic.Frame.h"
#include "Basic.Lock.h"
#include "Basic.ICompleter.h"
#include "Basic.IBufferedStream.h"

namespace Web
{
    using namespace Basic;
    using namespace Http;

    class Client : public Frame, public ICompleter, public std::enable_shared_from_this<Client>
    {
    public:
        enum State
        {
            inactive_state = Start_State,
            get_pending_state,
            resolve_address_state,
            connection_pending_state,
            headers_pending_state,
            body_pending_state,
            response_complete_state,
        };

    private:
        std::shared_ptr<IBufferedStream<byte> > peer;
        std::shared_ptr<IProcess> client_completion;
        ByteStringRef client_cookie;
        std::shared_ptr<ResponseHeadersFrame> response_headers_frame;
        std::shared_ptr<BodyFrame> response_body_frame;
        std::shared_ptr<MediaType> media_type;
        uint8 retries;
        uint8 redirects;
        std::shared_ptr<Request> planned_request;
        Lock lock;

        void switch_to_state(State state);
        void handle_error(const char* error);
        void Redirect(std::shared_ptr<Uri> url);
        void Retry(std::shared_ptr<Request> request);
        void QueuePlanned();
        void QueueJob();

        virtual void IProcess::consider_event(IEvent* event);

    public:
        TransactionList history;
        CookieList http_cookies;

        void Get(std::shared_ptr<Request> request, std::shared_ptr<IProcess> completion, ByteStringRef cookie);
        void Get(std::shared_ptr<Uri> url, std::shared_ptr<IProcess> completion, ByteStringRef cookie);

        virtual void ICompleter::complete(std::shared_ptr<void> context, uint32 count, uint32 error);

        bool get_content_type(std::shared_ptr<MediaType>* media_type);
        bool get_content_type_charset(UnicodeStringRef* media_type);
        void set_body_stream(std::shared_ptr<IStream<byte> > body_stream);
        void get_url(std::shared_ptr<Uri>* url);
    };
}