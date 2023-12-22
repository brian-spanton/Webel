// Copyright © 2013 Brian Spanton

#pragma once

#include "Http.ResponseHeadersFrame.h"
#include "Http.BodyFrame.h"
#include "Http.MediaType.h"
#include "Basic.Frame.h"
#include "Basic.Lock.h"
#include "Basic.ICompleter.h"
#include "Basic.IStream.h"

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
        std::shared_ptr<IStream<byte> > transport;
        std::weak_ptr<IProcess> completion;
        ByteStringRef completion_cookie;
        std::shared_ptr<ResponseHeadersFrame> response_headers_frame;
        std::shared_ptr<BodyFrame> response_body_frame;
        std::shared_ptr<MediaType> media_type;
        uint8 max_retries = 0;
        uint8 retries = 0;
        uint8 redirects = 0;
        std::shared_ptr<Request> planned_request;
        Lock lock;

        void switch_to_state(State state);
        void handle_error(const char* error);
        void Redirect(std::shared_ptr<Uri> url);
        void Retry(std::shared_ptr<Request> request);
        void QueuePlanned();
        void QueueJob();

        virtual EventResult IProcess::consider_event(IEvent* event);

    public:
        TransactionList history;
        CookieList http_cookies;

        void Get(std::shared_ptr<Request> request, uint8 max_retries, std::shared_ptr<IProcess> completion, ByteStringRef cookie);
        void Get(std::shared_ptr<Uri> url, uint8 max_retries, std::shared_ptr<IProcess> completion, ByteStringRef cookie);

        virtual void ICompleter::complete(std::shared_ptr<void> context, uint32 count, uint32 error);

        bool get_content_type(std::shared_ptr<MediaType>* media_type);
        bool get_content_type_charset(UnicodeStringRef* media_type);
        void set_decoded_content_stream(std::shared_ptr<IStream<byte> > body_stream);
        void get_url(std::shared_ptr<Uri>* url);
    };
}