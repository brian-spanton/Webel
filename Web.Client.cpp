// Copyright � 2013 Brian Spanton

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
#include "Basic.TextWriter.h"

namespace Web
{
    using namespace Basic;

    void Client::Get(std::shared_ptr<Uri> url, uint8 max_retries, std::shared_ptr<IProcess> completion, ByteStringRef cookie, bool is_iframe)
    {
        std::shared_ptr<Request> request = std::make_shared<Request>();
        request->Initialize();
        request->method = Http::globals->get_method;
        request->resource = url;
        request->is_iframe = is_iframe;

        Get(request, max_retries, completion, cookie);
    }

    void Client::Get(std::shared_ptr<Http::Request> request, uint8 max_retries, std::shared_ptr<IProcess> completion, ByteStringRef cookie)
    {
        // $$$ review how threading, eventing, job queueing and locking works with this class

        Hold hold(this->lock);

        if (get_state() != State::inactive_state)
            throw FatalError("Client::Get get_state() != State::inactive_state");

        this->retries = 0;
        this->redirects = 0;

        this->completion = completion;
        this->completion_cookie = cookie;

        this->planned_request = request;
        this->max_retries = max_retries;

        QueuePlanned();
    }

    void Client::complete(std::shared_ptr<void> context, uint32 count, uint32 error)
    {
        IoCompletionEvent event(context, count, error);
        process_event_ignore_failures(this, &event);
    }

    void Client::Redirect(std::shared_ptr<Uri> url)
    {
        if (this->redirects == 5)
        {
            handle_error("redirect error");
            this->planned_request = 0;
        }
        else
        {
            this->redirects++;
            this->retries = 0;

            std::shared_ptr<Request> request = std::make_shared<Request>();
            request->Initialize(this->transaction->request.get());
            request->resource = url;

            //$$$
            //Note: RFC1945 and RFC2068 specify that the client is not allowed
            //to change the method on the redirected request.  However, most
            //existing user agent implementations treat 302 as if it were a 303
            //response, performing a GET on the Location field-value regardless
            //of the original request method. The status codes 303 and 307 have
            //been added for servers that wish to make unambiguously clear which
            //kind of reaction is expected of the client.
            request->method = Http::globals->get_method;
            request->request_body = 0;

            this->planned_request = request;
        }
    }

    void Client::Retry(std::shared_ptr<Http::Request> request)
    {
        if (this->retries == this->max_retries)
        {
            handle_error("retry error");
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
            std::shared_ptr<IProcess> completion = this->completion.lock();
            if (completion.get() != 0)
            {
                ResponseCompleteEvent event;
                event.cookie = this->completion_cookie;
                process_event_ignore_failures(completion.get(), &event);
            }
        }
        else if (state == State::response_pending_state)
        {
            this->transaction = std::make_shared<Transaction>();
            this->transaction->request = this->planned_request;
            this->transaction->response = std::make_shared<Response>();
            this->transaction->response->Initialize();

            this->history.push_back(transaction);
            this->response_frame = std::make_shared<ResponseFrame>(this->transaction, this->shared_from_this(), ByteStringRef());

            this->planned_request->protocol = Http::globals->HTTP_1_1;

            if (this->planned_request->request_body.get() == 0)
            {
                this->planned_request->headers->set_base_10(Http::globals->header_content_length, 0);
                this->planned_request->headers->erase(Http::globals->header_content_type);
            }
            else
            {
                CountStream<byte> count_stream;
                this->planned_request->request_body->write_to_stream(&count_stream);
                this->planned_request->headers->set_base_10(Http::globals->header_content_length, count_stream.count);
            }

            this->planned_request->headers->set_string(Http::globals->header_te, Http::globals->trailers);
            this->planned_request->headers->set_string(Http::globals->header_host, planned_request->resource->host);

            UnicodeStringRef accept_type_value = std::make_shared<UnicodeString>();
            accept_type_value->append(*Http::globals->gzip);
            TextWriter writer(accept_type_value.get());
            writer.write_literal(", ");
            accept_type_value->append(*Http::globals->deflate);
            this->planned_request->headers->set_string(Http::globals->header_accept_type, accept_type_value);

            UnicodeStringRef cookie_header_value;

            for (CookieList::iterator it = this->http_cookies.begin(); it != this->http_cookies.end(); it++)
            {
                if ((*it)->Matches(this->planned_request->resource.get()))
                {
                    if (cookie_header_value.get() == 0)
                    {
                        cookie_header_value = std::make_shared<UnicodeString>();
                        cookie_header_value->reserve(0x400);
                    }
                    else
                    {
                        TextWriter writer(cookie_header_value.get());
                        writer.write_literal("; ");
                    }

                    std::shared_ptr<Cookie> cookie = (*it);

                    cookie_header_value->append(*cookie->name.get());
                    cookie_header_value->push_back(Http::globals->EQ);
                    cookie_header_value->append(*cookie->value.get());
                }
            }

            if (cookie_header_value.get() != 0)
                this->planned_request->headers->set_string(Http::globals->header_cookie, cookie_header_value);

            ByteString request_bytes;
            Http::serialize<Request>()(this->planned_request.get(), &request_bytes);
            request_bytes.write_to_stream(this->transport.get());

            render_request_line(this->planned_request.get(), &Basic::globals->DebugWriter()->decoder);
            Basic::globals->DebugWriter()->WriteLine();

            this->planned_request = 0;
        }
    }

    void Client::handle_error(const char* error)
    {
        HandleError(error);
    }

    void Client::QueueJob()
    {
        std::shared_ptr<Job> job = Job::make(this->shared_from_this(), std::shared_ptr<void>());
        Basic::globals->QueueJob(job);
    }

    void Client::QueuePlanned()
    {
        if (this->planned_request.get() != 0)
        {
            switch_to_state(State::get_pending_state);
            QueueJob();
        }
        else
        {
            switch_to_state(State::inactive_state);
        }
    }

    ProcessResult Client::consider_event(IEvent* event)
    {
        Hold hold(this->lock);

        if (event->get_type() == Basic::EventType::element_stream_ending_event)
            this->transport.reset();

        if (this->get_state() == State::inactive_state)
            return ProcessResult::process_result_blocked; // event consumed

        if (this->get_state() == State::response_pending_state)
        {
            if (event->get_type() == response_headers_event)
            {
                uint16 code = this->transaction->response->code;

                if (code / 100 == 3)
                {
                    UnicodeStringRef location_string;

                    bool success = this->transaction->response->headers->get_string(Http::globals->header_location, &location_string);
                    if (success)
                    {
                        std::shared_ptr<Uri> location_url = std::make_shared<Uri>();
                        location_url->Initialize();

                        bool success = location_url->Parse(location_string.get(), this->transaction->request->resource.get());
                        if (success)
                        {
                            this->transaction->request->resource->write_to_stream(Basic::globals->LogStream(), 0, 0);
                            Basic::globals->DebugWriter()->WriteFormat<24>(" redirected with %d to ", code);
                            location_url->write_to_stream(Basic::globals->LogStream(), 0, 0);
                            Basic::globals->DebugWriter()->WriteLine();

                            Redirect(location_url);
                        }
                    }
                }
                else if (code == 503)
                {
                    Retry(this->transaction->request);
                }

                // if we are not planning a retry or redirect, let the caller know headers are back, so it
                // can choose and set a body stream
                if (this->planned_request.get() == 0)
                {
                    std::shared_ptr<Uri> url;
                    get_url(&url);

                    url->write_to_stream(Basic::globals->LogStream(), 0, 0);
                    Basic::globals->DebugWriter()->WriteFormat<0x40>(" returned %d", this->transaction->response->code);
                    Basic::globals->DebugWriter()->WriteLine();

                    std::shared_ptr<IProcess> completion = this->completion.lock();
                    if (completion.get() != 0)
                    {
                        Http::ResponseHeadersEvent event;
                        event.cookie = this->completion_cookie;
                        process_event_ignore_failures(completion.get(), &event);
                    }
                }
            }
            else if (event->get_type() < response_headers_event)
            {
                ProcessResult result = process_event(this->response_frame.get(), event);
                if (result == process_result_blocked)
                    return ProcessResult::process_result_blocked;

                if (this->response_frame->failed())
                {
                    if (this->transport.get() != 0)
                    {
                        this->transport->write_eof();
                        this->transport.reset();
                    }

                    Retry(this->transaction->request);
                    QueuePlanned();
                    return ProcessResult::process_result_blocked; // event consumed, new thread
                }

                switch_to_state(State::response_complete_state);
                QueueJob();
            }

            return ProcessResult::process_result_blocked; // event consumed, new thread
        }

        if (event->get_type() == Basic::EventType::element_stream_ending_event)
        {
            switch (get_state())
            {
            case State::get_pending_state:
            case State::resolve_address_state:
            case State::response_complete_state:
                break;

            case State::connection_pending_state:
                Retry(this->planned_request);
                QueuePlanned();
                break;

            default:
                throw FatalError("Web::Client::handle_event unexpected state");
            }

            return ProcessResult::process_result_blocked; // event consumed
        }

        switch (get_state())
        {
        case State::get_pending_state:
            {
                if (event->get_type() != Basic::EventType::io_completion_event)
                {
                    HandleError("unexpected event");
                    return ProcessResult::process_result_blocked; // unexpected event
                }

                if (!this->planned_request->resource->is_http_scheme())
                {
                    handle_error("scheme error");
                    switch_to_state(State::inactive_state);
                    return ProcessResult::process_result_blocked; // event consumed
                }

                std::shared_ptr<Uri> current_url;
                if (this->transaction)
                    current_url = this->transaction->request->resource;

                if (this->transport &&
                    current_url &&
                    equals<UnicodeString, false>(this->planned_request->resource->scheme.get(), current_url->scheme.get()) &&
                    equals<UnicodeString, false>(this->planned_request->resource->host.get(), current_url->host.get()) &&
                    equals<UnicodeString, true>(this->planned_request->resource->port.get(), current_url->port.get()))
                {
                    // we already have a compatible transport, just go for it

                    switch_to_state(State::response_pending_state);
                }
                else
                {
                    if (this->transport.get() != 0)
                    {
                        this->transport->write_eof();
                        this->transport.reset();
                    }

                    std::shared_ptr<ClientSocket> client_socket;
                    Web::globals->CreateClientSocket(this->planned_request->resource->is_secure_scheme(), this->shared_from_this(), 0x400, &client_socket, &this->transport);

                    sockaddr_in addr;
                    bool success = client_socket->Resolve(this->planned_request->resource->host, this->planned_request->resource->get_port(), &addr);
                    if (!success)
                    {
                        handle_error("resolve failed");
                        switch_to_state(State::inactive_state);
                        return ProcessResult::process_result_blocked; // event consumed
                    }

                    client_socket->StartConnect(addr);

                    switch_to_state(State::connection_pending_state);
                }

                return ProcessResult::process_result_blocked; // event consumed
            }
            break;

        case State::connection_pending_state:
            {
                if (event->get_type() != Basic::EventType::can_send_bytes_event)
                {
                    HandleError("unexpected event");
                    return ProcessResult::process_result_blocked; // unexpected event
                }

                switch_to_state(State::response_pending_state);
                return ProcessResult::process_result_blocked; // event consumed
            }
            break;

        case State::response_complete_state:
            {
                if (event->get_type() != Basic::EventType::io_completion_event)
                {
                    HandleError("unexpected event");
                    return ProcessResult::process_result_blocked; // unexpected event
                }

                std::shared_ptr<Response> response = this->transaction->response;

                auto range = response->headers->equal_range(Http::globals->header_set_cookie);

                for (NameValueCollection::iterator it = range.first; it != range.second; it++)
                {
                    UnicodeStringRef cookie_value = it->second;

                    Basic::globals->DebugWriter()->write_literal("Cookie received: ");
                    cookie_value->write_to_stream(Basic::globals->LogStream());
                    Basic::globals->DebugWriter()->WriteLine();

                    // $ conform to RFC6265 section 5.3 (storage model)

                    std::shared_ptr<Cookie> cookie = std::make_shared<Cookie>();
                    cookie->Initialize(cookie_value.get());

                    bool found = false;
                    for (CookieList::iterator it = this->http_cookies.begin(); it != this->http_cookies.end(); it++)
                    {
                        if (cookie->equals(it->get()))
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
                return ProcessResult::process_result_blocked; // event consumed, new thread
            }
            break;

        default:
            throw FatalError("Web::Client::handle_event unexpected state");
        }

        return ProcessResult::process_result_ready;
    }

    bool Client::get_content_type(std::shared_ptr<MediaType>* media_type)
    {
        Hold hold(this->lock);

        UnicodeStringRef content_type;

        bool success = this->transaction->response->headers->get_string(Http::globals->header_content_type, &content_type);
        if (!success)
            return false;

        (*media_type) = std::make_shared<MediaType>();
        (*media_type)->Initialize(content_type.get());

        return true;
    }

    bool Client::get_content_type_charset(UnicodeStringRef* charset)
    {
        std::shared_ptr<MediaType> content_type;
        bool success = this->get_content_type(&content_type);
        if (!success)
            return false;

        success = content_type->parameters->get_string(Basic::globals->charset_parameter_name, charset);
        if (!success)
            return false;

        return true;
    }

    void Client::set_decoded_content_stream(std::shared_ptr<IStream<byte> > decoded_content_stream)
    {
        Hold hold(this->lock);

        this->response_frame->set_decoded_content_stream(decoded_content_stream);
    }

    void Client::get_url(std::shared_ptr<Uri>* url)
    {
        Hold hold(this->lock);

        (*url) = this->transaction->request->resource;
    }
}