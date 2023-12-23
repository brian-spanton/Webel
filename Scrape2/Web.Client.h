// Copyright © 2013 Brian Spanton

#pragma once

#include "Http.ResponseHeadersFrame.h"
#include "Http.BodyFrame.h"
#include "Http.MediaType.h"
#include "Basic.StateMachine.h"
#include "Basic.Lock.h"
#include "Basic.IJobEventHandler.h"
#include "Basic.ITransportEventHandler.h"
#include "Basic.IStream.h"
#include "Web.IClientEventHandler.h"

namespace Web
{
    using namespace Basic;
    using namespace Http;

    class Client : public StateMachine, public ITransportEventHandler<byte>, public IJobEventHandler, public std::enable_shared_from_this<Client>
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
        std::weak_ptr<IClientEventHandler> event_handler;
        ByteStringRef event_handler_cookie;
        std::shared_ptr<ResponseHeadersFrame> response_headers_frame;
        std::shared_ptr<BodyFrame> response_body_frame;
        std::shared_ptr<MediaType> media_type;
        uint8 max_retries;
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

        void consider_event(bool job_completed, bool transport_received, byte element, bool write_elements, const byte* elements, uint32 count, bool write_eof, bool transport_connected);

    public:
        TransactionList history;
        CookieList http_cookies;

        void Get(std::shared_ptr<Request> request, uint8 max_retries, std::shared_ptr<IClientEventHandler> event_handler, ByteStringRef event_handler_cookie);
        void Get(std::shared_ptr<Uri> url, uint8 max_retries, std::shared_ptr<IClientEventHandler> event_handler, ByteStringRef event_handler_cookie);

        virtual void IJobEventHandler::job_completed(std::shared_ptr<void> context, uint32 count, uint32 error);
		virtual void ITransportEventHandler<byte>::transport_received(const byte* elements, uint32 count);
		virtual void ITransportEventHandler<byte>::transport_disconnected();
        virtual void ITransportEventHandler<byte>::transport_connected();

        bool get_content_type(std::shared_ptr<MediaType>* media_type);
        bool get_content_type_charset(UnicodeStringRef* media_type);
        void set_body_stream(std::shared_ptr<IStream<byte> > body_stream);
        void get_url(std::shared_ptr<Uri>* url);
    };
}