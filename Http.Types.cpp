// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Http.Types.h"
#include "Http.CookieParser.h"
#include "Http.Globals.h"
#include "Basic.Globals.h"

namespace Http
{
    using namespace Basic;

    void Request::Initialize()
    {
        this->method = New<UnicodeString>();

        this->resource = New<Uri>();
        this->resource->Initialize();

        this->protocol = New<UnicodeString>();
        this->headers = New<NameValueCollection >();
    }

    void Request::Initialize(Request* request)
    {
        this->method = request->method;
        this->resource = request->resource;
        this->protocol = request->protocol;
        this->headers = request->headers;
        this->client_body = request->client_body;
    }

    void Response::Initialize()
    {
        this->protocol = New<UnicodeString>();
        this->code = 0;
        this->reason = New<UnicodeString>();
        this->headers = New<NameValueCollection >();
    }

    void Cookie::Initialize()
    {
        this->name = New<UnicodeString>();
        this->value = New<UnicodeString>();
        this->secure_only_flag = false;
        this->http_only_flag = false;
        this->host_only_flag = false;
    }

    void Cookie::Initialize(UnicodeString* value)
    {
        Initialize();

        Inline<CookieParser> frame;
        frame.Initialize(this);

        frame.Write(value->c_str(), value->size());
        frame.WriteEOF();
    }

    bool Cookie::equals(Cookie* value)
    {
        if (!value->name.equals<true>(this->name))
            return false;

        if (!value->domain.equals<true>(this->domain))
            return false;

        if (!value->path.equals<true>(this->path))
            return false;

        return true;
    }

    bool Cookie::Matches(Uri* url)
    {
        Path host_path;

        UnicodeString::Ref node = New<UnicodeString>();

        for (uint32 i = 0; i < url->host->size(); i++)
        {
            Codepoint c = url->host->at(i);

            if (c == '.')
            {
                host_path.insert(host_path.begin(), node);
                node = New<UnicodeString>();
            }
            else
            {
                node->push_back(c);
            }
        }

        host_path.insert(host_path.begin(), node);
        node = New<UnicodeString>();

        if (this->host_only_flag)
        {
            if (!host_path.equals<false>(this->domain))
                return false;
        }
        else
        {
            if (!host_path.BelongsTo<false>(this->domain))
                return false;
        }

        if (!url->path.BelongsTo<false>(this->path))
            return false;

        if (this->secure_only_flag)
        {
            if (!url->is_secure_scheme())
                return false;
        }

        if (this->http_only_flag)
        {
            if (!url->is_http_scheme())
                return false;
        }

        return true;
    }

    uint32 ResponseHeadersEvent::get_type()
    {
        return EventType::response_headers_event;
    }

    uint32 ResponseCompleteEvent::get_type()
    {
        return EventType::response_complete_event;
    }

    uint32 AcceptCompleteEvent::get_type()
    {
        return EventType::accept_complete_event;
    }
}