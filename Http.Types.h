// Copyright � 2013 Brian Spanton

#pragma once

#include "Basic.NameValueCollection.h"
#include "Basic.Uri.h"
#include "Basic.IStreamWriter.h"

namespace Http
{
    using namespace Basic;

    struct Request
    {
        UnicodeStringRef method; // $$ unicode?!
        std::shared_ptr<Uri> resource;
        UnicodeStringRef protocol; // $$ unicode?!
        std::shared_ptr<NameValueCollection> headers;
        std::shared_ptr<IStreamWriter<byte> > request_body;
        bool is_iframe = false;

        void Initialize();
        void Initialize(Request* request);

        void render_request_line(IStream<Codepoint>* stream);
        void render_request_line(IStream<byte>* stream);
    };

    struct Response
    {
        UnicodeStringRef protocol; // $$ unicode?!
        uint16 code = 0;
        UnicodeStringRef reason; // $$ unicode?!
        std::shared_ptr<NameValueCollection> headers;
        std::shared_ptr<IStreamWriter<byte> > response_body;

        void Initialize();

        void render_response_line(IStream<Codepoint>* stream);
        void render_response_line(IStream<byte>* stream);
    };

    struct Transaction
    {
        std::shared_ptr<Request> request;
        std::shared_ptr<Response> response;
    };

    typedef std::vector<std::shared_ptr<Transaction> > TransactionList;

    struct Cookie
    {
        UnicodeStringRef name;
        UnicodeStringRef value;
        // $ DateTime expire_time;
        Path domain;
        Path path;
        // $ DateTime creation_time;
        // $ DateTime last_access_time;
        bool persistent_flag = false;
        bool host_only_flag = false;
        bool secure_only_flag = false;
        bool http_only_flag = false;

        void Initialize();
        void Initialize(UnicodeString* value);

        bool Matches(Uri* url);
        bool equals(Cookie* value);
    };

    typedef std::vector<std::shared_ptr<Cookie> > CookieList;

    enum EventType
    {
        response_headers_event = 0x1000,
        response_complete_event,
        accept_complete_event,
    };

    struct ResponseHeadersEvent : public Basic::ContextualizedEvent
    {
        virtual uint32 get_type();
    };

    struct ResponseCompleteEvent : public Basic::ContextualizedEvent
    {
        virtual uint32 get_type();
    };

    struct AcceptCompleteEvent : public Basic::ContextualizedEvent
    {
        virtual uint32 get_type();
    };

    ///////////////////////////////////////////////////////////////////////////
    // serialization meta template
    ///////////////////////////////////////////////////////////////////////////

    template <typename value_type>
    struct __declspec(novtable) serialize
    {
        void operator()(const value_type* value, IStream<byte>* stream) const
        {
        	static_assert(false, "No Http::serialize defined for this type");
        }
    };
}