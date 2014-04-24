// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Web.Client.h"
#include "Basic.ClientSocket.h"
#include "Basic.Globals.h"
#include "Basic.CountStream.h"
#include "Http.CookieParser.h"
#include "Http.Globals.h"
#include "Http.RequestFrame.h"
#include "Basic.SingleByteDecoder.h"
#include "Basic.SingleByteEncodingIndex.h"
#include "Web.Globals.h"

namespace Web
{
    using namespace Basic;

    void Client::Initialize()
    {
        __super::Initialize();
        this->peer = 0;
    }

    void Client::Process(IEvent* event)
    {
        Hold hold(this->lock);
        __super::Process(event);
    }

    void Client::Get(Uri* url, Basic::Ref<IProcess> completion, ByteString::Ref cookie)
    {
        Hold hold(this->lock);

        Request::Ref request = New<Request>();
        request->Initialize();
        request->method = Http::globals->get_method;
        request->resource = url;

        Get(request, completion, cookie);
    }

    void Client::Get(Http::Request* request, Basic::Ref<IProcess> completion, ByteString::Ref cookie)
    {
        Hold hold(this->lock);

        if (frame_state() != State::inactive_state)
            throw new Exception("Client::Get frame_state() != State::inactive_state");

        this->retries = 0;
        this->redirects = 0;

        this->client_completion = completion;
        this->client_cookie = cookie;

        this->planned_request = request;

        switch_to_state(State::get_pending_state);
        Basic::globals->QueueProcess(this, (ByteString*)0);
    }

    void Client::Redirect(Http::Uri* url)
    {
        if (this->redirects == 5)
        {
            Error("redirect error");
            this->planned_request = 0;
        }
        else
        {
            this->redirects++;
            this->retries = 0;

            Request::Ref request = New<Request>();
            request->Initialize(this->history.back().request);
            request->resource = url;

            //Note: RFC 1945 and RFC 2068 specify that the client is not allowed
            //to change the method on the redirected request.  However, most
            //existing user agent implementations treat 302 as if it were a 303
            //response, performing a GET on the Location field-value regardless
            //of the original request method. The status codes 303 and 307 have
            //been added for servers that wish to make unambiguously clear which
            //kind of reaction is expected of the client.
            request->method = Http::globals->get_method;
            request->client_body = 0;

            this->planned_request = request;
        }
    }

    void Client::Retry(Http::Request* request)
    {
        if (this->retries == 0) // $$ was 2
        {
            Error("retry error");
            this->planned_request = 0;
        }
        else
        {
            this->retries++;
            this->planned_request = request;
        }
    }

    void Client::switch_to_state(State state)
    {
        __super::switch_to_state(state);

        if (state == State::inactive_state)
        {
            Basic::globals->DebugWriter()->WriteLine("Complete");

            Basic::Ref<IProcess> completion = this->client_completion;
            this->client_completion = 0;

            ResponseCompleteEvent event;
            event.cookie = this->client_cookie;
            this->client_cookie = 0;

            if (completion.item() != 0)
                completion->Process(&event);
        }
        else if (state == State::headers_pending_state)
        {
            Transaction transaction;
            transaction.request = this->planned_request;
            transaction.response = New<Response>();
            transaction.response->Initialize();

            this->history.push_back(transaction);
            this->response_headers_frame.Initialize(transaction.request->method, transaction.response);

            this->planned_request->protocol = Http::globals->HTTP_1_1;

            if (this->planned_request->client_body.item() == 0)
            {
                this->planned_request->headers->set_base_10(Http::globals->header_content_length, 0);
                this->planned_request->headers->erase(Http::globals->header_content_type);
            }
            else
            {
                Inline<CountStream<byte> > count_stream;
                this->planned_request->client_body->SerializeTo(&count_stream);
                this->planned_request->headers->set_base_10(Http::globals->header_content_length, count_stream.count);
            }

            this->planned_request->headers->set_string(Http::globals->header_te, Http::globals->trailers);
            this->planned_request->headers->set_string(Http::globals->header_host, planned_request->resource->host);

            UnicodeString::Ref cookie_header_value;

            for (CookieList::iterator it = this->http_cookies.begin(); it != this->http_cookies.end(); it++)
            {
                if ((*it)->Matches(this->planned_request->resource))
                {
                    if (cookie_header_value.item() == 0)
                    {
                        cookie_header_value = New<UnicodeString>();
                        cookie_header_value->reserve(0x400);
                    }
                    else
                    {
                        TextWriter writer(cookie_header_value);
                        writer.Write("; ");
                    }

                    Cookie::Ref cookie = (*it);

                    cookie_header_value->append(*cookie->name.item());
                    cookie_header_value->push_back(Http::globals->EQ);
                    cookie_header_value->append(*cookie->value.item());
                }
            }

            if (cookie_header_value.item() != 0)
                this->planned_request->headers->set_string(Http::globals->header_cookie, cookie_header_value);

            Inline<RequestFrame> requestFrame;
            requestFrame.Initialize(this->planned_request);
            requestFrame.SerializeTo(this->peer);

            this->peer->Flush();

            Inline<ByteString> request_bytes;
            requestFrame.SerializeTo(&request_bytes);

            this->planned_request = 0;

            Basic::globals->DebugWriter()->Write("Request sent: ");
            Basic::globals->DebugWriter()->Write((const char*)request_bytes.c_str(), request_bytes.size());
            Basic::globals->DebugWriter()->WriteLine();
        }
    }

    void Client::Error(const char* error)
    {
        HandleError(error);
    }

    void Client::QueuePlanned()
    {
        if (this->planned_request.item() != 0)
        {
            switch_to_state(State::get_pending_state);
            Basic::globals->QueueProcess(this, (ByteString*)0);
        }
        else
        {
            switch_to_state(State::inactive_state);
        }
    }

    void Client::Process(IEvent* event, bool* yield)
    {
        Hold hold(this->lock);

        (*yield) = true;

        switch (frame_state())
        {
        case State::inactive_state:
            switch (event->get_type())
            {
            case Basic::EventType::element_stream_ending_event:
                break;

            default:
                throw new Exception("unexpected event");
            }
            break;

        case State::get_pending_state:
            switch (event->get_type())
            {
            case Basic::EventType::element_stream_ending_event:
                break;

            case Basic::EventType::process_event:
                {
                    if (!this->planned_request->resource->is_http_scheme())
                    {
                        Error("scheme error");
                        switch_to_state(State::inactive_state);
                        return;
                    }

                    Uri::Ref current_url;
                    if (this->history.size() > 0)
                        current_url = this->history.back().request->resource;

                    if (!(this->peer.item() != 0 &&
                        current_url.item() != 0 &&
                        this->planned_request->resource->scheme.equals<false>(current_url->scheme) && 
                        this->planned_request->resource->host.equals<false>(current_url->host) && 
                        this->planned_request->resource->port.equals<true>(current_url->port)))
                    {
                        if (this->peer.item() != 0)
                        {
                            this->peer->WriteEOF();
                            this->peer = 0;
                        }

                        ClientSocket::Ref client_socket;
                        Web::globals->CreateClientSocket(this->planned_request->resource->is_secure_scheme(), this, &client_socket, &this->peer);

                        sockaddr_in addr;
                        bool success = client_socket->Resolve(this->planned_request->resource->host, this->planned_request->resource->get_port(), &addr);
                        if (!success)
                        {
                            Error("resolve failed");
                            switch_to_state(State::inactive_state);
                            return;
                        }

                        client_socket->StartConnect(addr);

                        switch_to_state(State::connection_pending_state);
                    }
                    else
                    {
                        switch_to_state(State::headers_pending_state);
                    }
                }
                break;

            default:
                throw new Exception("unexpected event");
            }
            break;

        case State::connection_pending_state:
            switch (event->get_type())
            {
            case Basic::EventType::element_stream_ending_event:
                Retry(this->planned_request);
                QueuePlanned();
                break;

            case Basic::EventType::ready_for_write_bytes_event:
                switch_to_state(State::headers_pending_state);
                break;

            default:
                throw new Exception("unexpected event");
            }
            break;

        case State::headers_pending_state:
            switch (event->get_type())
            {
            case Basic::EventType::element_stream_ending_event:
                Retry(this->history.back().request);
                QueuePlanned();
                break;

            case Basic::EventType::ready_for_read_bytes_event:
                if (this->response_headers_frame.Pending())
                {
                    this->response_headers_frame.Frame::Process(event);
                }

                if (this->response_headers_frame.Failed())
                {
                    Error("response_headers_frame failed");
                    switch_to_state(State::inactive_state);
                }
                else if (this->response_headers_frame.Succeeded())
                {
                    (*yield) = false;

                    Basic::globals->DebugWriter()->Write("Response received: ");
                    Inline<SingleByteDecoder> decoder;
                    decoder.Initialize(Basic::globals->ascii_index, Basic::globals->DebugStream());
                    this->response_headers_frame.SerializeTo(&decoder);

                    Response::Ref response = this->history.back().response;
                    uint16 code = response->code;

                    if (code / 100 == 3)
                    {
                        UnicodeString::Ref location_string;

                        bool success = response->headers->get_string(Http::globals->header_location, &location_string);
                        if (success)
                        {
                            Uri::Ref location_url = New<Uri>();
                            location_url->Initialize();

                            bool success = location_url->Parse(location_string, this->history.back().request->resource);
                            if (success)
                            {
                                Redirect(location_url);
                            }
                        }
                    }
                    else if (code == 503)
                    {
                        Retry(this->history.back().request);
                    }

                    if (this->planned_request.item() == 0)
                    {
                        Http::ResponseHeadersEvent event;
                        this->client_completion->Process(&event);
                    }

                    if (code / 100 == 1 || code == 204 || code == 205 || code == 304 || this->history.back().request->method.equals<true>(Http::globals->head_method))
                    {
                        switch_to_state(State::response_complete_state);
                    }
                    else
                    {
                        this->response_body_frame.Initialize(this->history.back().response->headers);
                        switch_to_state(State::body_pending_state);
                    }
                }
                break;

            default:
                throw new Exception("unexpected event");
            }
            break;

        case State::body_pending_state:
            switch (event->get_type())
            {
            case Basic::EventType::element_stream_ending_event:
            case Basic::EventType::ready_for_read_bytes_event:
                if (this->response_body_frame.Pending())
                {
                    this->response_body_frame.Frame::Process(event);
                }
                
                if (this->response_body_frame.Failed())
                {
                    Retry(this->history.back().request);
                    QueuePlanned();
                }
                else if (this->response_body_frame.Succeeded())
                {
                    (*yield) = false;
                    switch_to_state(State::response_complete_state);
                }
                break;

            default:
                throw new Exception("unexpected event");
            }
            break;

        case State::response_complete_state:
            {
                Response::Ref response = this->history.back().response;

                NameValueCollection::_Pairii range = response->headers->equal_range(Http::globals->header_set_cookie);

                for (NameValueCollection::iterator it = range.first; it != range.second; it++)
                {
                    UnicodeString::Ref cookie_value = it->second;

                    Basic::globals->DebugWriter()->Write("Cookie received: ");
                    cookie_value->write_to(Basic::globals->DebugStream());
                    Basic::globals->DebugWriter()->WriteLine();

                    // $ conform to RFC6265 section 5.3 (storage model)

                    Cookie::Ref cookie = New<Cookie>();
                    cookie->Initialize(cookie_value);

                    bool found = false;
                    for (CookieList::iterator it = this->http_cookies.begin(); it != this->http_cookies.end(); it++)
                    {
                        if (cookie->equals(*it))
                        {
                            found = true;

                            // 3. Update the creation-time of the newly created cookie to
                            //    match the creation-time of the old-cookie.
                            // $ NYI

                            this->http_cookies.erase(it);
                            break;
                        }
                    }

                    this->http_cookies.push_back(cookie);
                }

                QueuePlanned();
            }
            break;

        default:
            throw new Exception("Web::Client::Process unexpected state");
        }
    }

    bool Client::get_content_type(MediaType::Ref* media_type)
    {
        Hold hold(this->lock);

        UnicodeString::Ref content_type;

        bool success = this->history.back().response->headers->get_string(Http::globals->header_content_type, &content_type);
        if (!success)
            return false;

        (*media_type) = New<MediaType>();
        (*media_type)->Initialize(content_type);

        return true;
    }

    bool Client::get_content_type_charset(UnicodeString::Ref* charset)
    {
        MediaType::Ref content_type;
        bool success = this->get_content_type(&content_type);
        if (!success)
            return false;

        success = content_type->parameters->get_string(Basic::globals->charset_parameter_name, charset);
        if (!success)
            return false;

        return true;
    }

    void Client::set_body_stream(IStream<byte>* body_stream)
    {
        this->response_body_frame.set_body_stream(body_stream);
    }

    void Client::get_url(Uri::Ref* url)
    {
        (*url) = this->history.back().request->resource;
    }
}