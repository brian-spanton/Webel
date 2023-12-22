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
#include "Http.ResponseHeadersFrame.h"

namespace Web
{
    using namespace Basic;

    void Client::Get(std::shared_ptr<Uri> url, uint8 max_retries, std::shared_ptr<IProcess> completion, ByteStringRef cookie)
    {
        std::shared_ptr<Request> request = std::make_shared<Request>();
        request->Initialize();
        request->method = Http::globals->get_method;
        request->resource = url;

        Get(request, max_retries, completion, cookie);
    }

    void Client::Get(std::shared_ptr<Http::Request> request, uint8 max_retries, std::shared_ptr<IProcess> completion, ByteStringRef cookie)
    {
        // $$$ reconsider how threading, eventing, job queueing and locking works with this class

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
        ProcessEvent event;
        produce_event(this, &event);
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
            request->Initialize(this->history.back().request.get());
            request->resource = url;

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
                produce_event(completion.get(), &event);
            }
        }
        else if (state == State::headers_pending_state)
        {
            Transaction transaction;
            transaction.request = this->planned_request;
            transaction.response = std::make_shared<Response>();
            transaction.response->Initialize();

            this->history.push_back(transaction);
            this->response_headers_frame = std::make_shared<ResponseHeadersFrame>(transaction.request->method, transaction.response.get());

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

            Basic::globals->DebugWriter()->write_literal("Request sent: ");
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

    EventResult Client::consider_event(IEvent* event)
    {
        Hold hold(this->lock);

        if (get_state() == State::inactive_state)
            return EventResult::event_result_yield; // event consumed

        if (event->get_type() == Basic::EventType::element_stream_ending_event)
        {
            this->transport.reset();

            switch (get_state())
            {
            case State::get_pending_state:
            case State::resolve_address_state:
                return EventResult::event_result_yield; // event consumed

            case State::connection_pending_state:
            case State::headers_pending_state:
                Retry(this->planned_request);
                QueuePlanned();
                return EventResult::event_result_yield; // event consumed, new thread

            case State::body_pending_state:
                {
                    EventResult result = delegate_event(this->response_body_frame.get(), event);
                    if (result == event_result_yield)
                        return EventResult::event_result_yield;

                    if (this->response_body_frame->failed())
                    {
                        Retry(this->history.back().request);
                        QueuePlanned();
                        return EventResult::event_result_yield; // event consumed, new thread
                    }

                    switch_to_state(State::response_complete_state);
                    QueueJob();
                    return EventResult::event_result_yield; // event consumed
                }
                break;

            case State::response_complete_state:
                return EventResult::event_result_yield; // event consumed

            default:
                throw FatalError("Web::Client::handle_event unexpected state");
            }
        }

        switch (get_state())
        {
        case State::get_pending_state:
            {
                if (event->get_type() != Basic::EventType::process_event)
                {
                    HandleError("unexpected event");
                    return EventResult::event_result_yield; // unexpected event
                }

                if (!this->planned_request->resource->is_http_scheme())
                {
                    handle_error("scheme error");
                    switch_to_state(State::inactive_state);
                    return EventResult::event_result_yield; // event consumed
                }

                std::shared_ptr<Uri> current_url;
                if (this->history.size() > 0)
                    current_url = this->history.back().request->resource;

                if (!(this->transport.get() != 0 &&
                    current_url.get() != 0 &&
                    equals<UnicodeString, false>(this->planned_request->resource->scheme.get(), current_url->scheme.get()) && 
                    equals<UnicodeString, false>(this->planned_request->resource->host.get(), current_url->host.get()) && 
                    equals<UnicodeString, true>(this->planned_request->resource->port.get(), current_url->port.get())))
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
                        return EventResult::event_result_yield; // event consumed
                    }

                    client_socket->StartConnect(addr);

                    switch_to_state(State::connection_pending_state);
                }
                else
                {
                    switch_to_state(State::headers_pending_state);
                }

                return EventResult::event_result_yield; // event consumed
            }
            break;

        case State::connection_pending_state:
            {
                if (event->get_type() != Basic::EventType::can_send_bytes_event)
                {
                    HandleError("unexpected event");
                    return EventResult::event_result_yield; // unexpected event
                }

                switch_to_state(State::headers_pending_state);
                return EventResult::event_result_yield; // event consumed
            }
            break;

        case State::headers_pending_state:
            {
                if (event->get_type() != Basic::EventType::received_bytes_event)
                {
                    HandleError("unexpected event");
                    return EventResult::event_result_yield; // unexpected event
                }

                EventResult result = delegate_event(this->response_headers_frame.get(), event);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                if (this->response_headers_frame->failed())
                {
                    handle_error("response_headers_frame failed");
                    switch_to_state(State::inactive_state);
                    return EventResult::event_result_yield; // event consumed
                }

                std::shared_ptr<Response> response = this->history.back().response;

                Basic::globals->DebugWriter()->write_literal("Response received: ");
                render_response_line(response.get(), &Basic::globals->DebugWriter()->decoder);
                Basic::globals->DebugWriter()->WriteLine();

                uint16 code = response->code;

                if (code / 100 == 3)
                {
                    UnicodeStringRef location_string;

                    bool success = response->headers->get_string(Http::globals->header_location, &location_string);
                    if (success)
                    {
                        std::shared_ptr<Uri> location_url = std::make_shared<Uri>();
                        location_url->Initialize();

                        bool success = location_url->Parse(location_string.get(), this->history.back().request->resource.get());
                        if (success)
                        {
                            this->history.back().request->resource->write_to_stream(Basic::globals->LogStream(), 0, 0);
                            Basic::globals->DebugWriter()->WriteFormat<24>(" redirected with %d to ", code);
                            location_url->write_to_stream(Basic::globals->LogStream(), 0, 0);
                            Basic::globals->DebugWriter()->WriteLine();

                            Redirect(location_url);
                        }
                    }
                }
                else if (code == 503)
                {
                    Retry(this->history.back().request);
                }

                bool body_expected = !(code / 100 == 1 || code == 204 || code == 205 || code == 304 || equals<UnicodeString, true>(this->history.back().request->method.get(), Http::globals->head_method.get()));

                if (body_expected)
                {
                    this->response_body_frame = std::make_shared<BodyFrame>(this->history.back().response->headers);
                }

                if (this->planned_request.get() == 0)
                {
                    std::shared_ptr<IProcess> completion = this->completion.lock();
                    if (completion.get() != 0)
                    {
                        Http::ResponseHeadersEvent event;
                        event.cookie = this->completion_cookie;
                        produce_event(completion.get(), &event);
                    }
                }

                if (!body_expected)
                {
                    switch_to_state(State::response_complete_state);
                    QueueJob();
                    return EventResult::event_result_yield; // event consumed, new thread
                }

                switch_to_state(State::body_pending_state);
            }
            break;

        case State::body_pending_state:
            {
                if (event->get_type() != Basic::EventType::received_bytes_event)
                    throw FatalError("unexpected event");

                EventResult result = delegate_event(this->response_body_frame.get(), event);
                if (result == event_result_yield)
                    return EventResult::event_result_yield;

                if (this->response_body_frame->failed())
                {
                    // consume all remaining bytes (if any)
                    Event::Read<byte>(event, 0xffffffff, 0, 0);

                    Retry(this->history.back().request);
                    QueuePlanned();
                    return EventResult::event_result_yield; // event consumed, new thread
                }

                switch_to_state(State::response_complete_state);
                QueueJob();
                return EventResult::event_result_yield; // event consumed, new thread
            }
            break;

        case State::response_complete_state:
            {
                if (event->get_type() != Basic::EventType::process_event)
                {
                    HandleError("unexpected event");
                    return EventResult::event_result_yield; // unexpected event
                }

                std::shared_ptr<Response> response = this->history.back().response;

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
                return EventResult::event_result_yield; // event consumed, new thread
            }
            break;

        default:
            throw FatalError("Web::Client::handle_event unexpected state");
        }

        return EventResult::event_result_continue;
    }

    bool Client::get_content_type(std::shared_ptr<MediaType>* media_type)
    {
        Hold hold(this->lock);

        UnicodeStringRef content_type;

        bool success = this->history.back().response->headers->get_string(Http::globals->header_content_type, &content_type);
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

        this->response_body_frame->set_decoded_content_stream(decoded_content_stream);
    }

    void Client::get_url(std::shared_ptr<Uri>* url)
    {
        Hold hold(this->lock);

        (*url) = this->history.back().request->resource;
    }
}