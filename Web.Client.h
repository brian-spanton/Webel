// Copyright � 2013 Brian Spanton

#pragma once

#include "Http.ResponseFrame.h"
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
            response_pending_state,
            body_pending_state,
            response_complete_state,
        };

    private:
        TransactionList history;
        std::shared_ptr<IStream<byte> > transport;
        std::weak_ptr<IProcess> call_back;
        std::shared_ptr<void> context;
        std::shared_ptr<ResponseFrame> response_frame;
        uint8 max_retries = 0;
        uint8 retries = 0;
        uint8 redirects = 0;
        std::shared_ptr<Request> planned_request;
        Lock lock;

        void Redirect(std::shared_ptr<Uri> url);
        void Retry(std::shared_ptr<Request> request);
        void QueuePlanned();
        void QueueJob();
        void Completion();
        void SendRequest();
        bool is_transport_reusable();

        virtual ProcessResult IProcess::process_event(IEvent* event);

    public:
        // l$$ these members being public makes them accessible without taking the lock... sketchy
        std::shared_ptr<Transaction> transaction;
        CookieList http_cookies;

        void Get(std::shared_ptr<Uri> url, uint8 max_retries, std::shared_ptr<IProcess> call_back, std::shared_ptr<void> context, bool is_iframe = false);
        void Get(std::shared_ptr<Request> request, uint8 max_retries, std::shared_ptr<IProcess> call_back, std::shared_ptr<void> context);

        virtual void ICompleter::complete(std::shared_ptr<void> context, uint32 count, uint32 error);

        bool get_content_type(std::shared_ptr<MediaType>* media_type);
        bool get_content_type_charset(UnicodeStringRef* media_type);
        void set_decoded_content_stream(std::shared_ptr<IStream<byte> > body_stream);
        void get_url(std::shared_ptr<Uri>* url);
    };
}