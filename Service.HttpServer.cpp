// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Service.HttpServer.h"
#include "Basic.ServerSocket.h"
#include "Basic.FrameStream.h"
#include "Basic.LogStream.h"
#include "Basic.CountStream.h"
#include "Http.Globals.h"
#include "Http.ResponseHeadersFrame.h"
#include "Http.HeadersFrame.h"
#include "Html.ElementNode.h"
#include "Service.Globals.h"

namespace Service
{
    using namespace Basic;

    HttpServer::HttpServer(std::shared_ptr<IProcess> completion, ByteStringRef cookie) :
        Server(completion, cookie)
    {
    }

    void HttpServer::handle_event()
    {
        if (this->request->resource->path.size() > 0)
        {
            UnicodeStringRef resource = this->request->resource->path.at(0);

            if (equals<UnicodeString, false>(resource.get(), Service::globals->root_admin.get()))
            {
                AdminRequest(this->request.get(), this->response.get());
            }
            else if (equals<UnicodeString, false>(resource.get(), Service::globals->root_echo.get()))
            {
                EchoRequest(this->request.get(), this->response.get());
            }
            else if (equals<UnicodeString, false>(resource.get(), Service::globals->root_question.get()))
            {
                QuestionRequest(this->request.get(), this->response.get());
            }
            else
            {
                this->response->code = 404;
                this->response->reason = Http::globals->reason_request_uri;
            }
        }
        else
        {
            this->response->code = 404;
            this->response->reason = Http::globals->reason_request_uri;
        }
    }

    void HttpServer::AdminRequest(Request* request, Response* response)
    {
        if (equals<UnicodeString, true>(request->method.get(), Http::globals->get_method.get()))
        {
            // $ should send it back chunked instead of buffering
            UnicodeStringRef response_body = std::make_shared<UnicodeString>();

            Service::globals->adminProtocol->reset(response_body);

            FrameStream<Codepoint> frame_stream;
            frame_stream.Initialize(Service::globals->adminProtocol.get());

            if (request->resource->query.get() != 0)
            {
                StringMap data_set;
                Web::Form::url_decode(request->resource->query, UnicodeStringRef(), false, &data_set);
                frame_stream.write_elements(data_set.begin()->second->address(), data_set.begin()->second->size());
            }

            frame_stream.write_element(Http::globals->CR);
            frame_stream.write_eof();

            ByteStringRef encoded = std::make_shared<ByteString>();
            utf_8_encode(response_body.get(), encoded.get());

            // $ set charset in mediatype

            response->server_body = encoded;
            response->headers->set_string(Http::globals->header_content_type, Basic::globals->text_plain_media_type);
            response->code = 200;
            response->reason = Http::globals->reason_ok;
        }
        else
        {
            response->code = 405;
            response->reason = Http::globals->reason_method;
        }
    }

    void HttpServer::EchoRequest(Request* request, Response* response)
    {
        if (equals<UnicodeString, true>(request->method.get(), Http::globals->get_method.get()))
        {
            ByteStringRef body = std::make_shared<ByteString>();
            body->reserve(0x400);

            Http::serialize<Request>()(request, body.get());

            response->server_body = body;
            response->headers->set_string(Http::globals->header_content_type, Basic::globals->text_plain_media_type);
            response->code = 200;
            response->reason = Http::globals->reason_ok;
        }
        else
        {
            response->code = 405;
            response->reason = Http::globals->reason_method;
        }
    }

    void HttpServer::QuestionRequest(Request* request, Response* response)
    {
        if (equals<UnicodeString, true>(request->method.get(), Http::globals->get_method.get()))
        {
            UnicodeStringRef response_body = std::make_shared<UnicodeString>();
            response_body->reserve(0x400);

            TextWriter writer(response_body.get());
            writer.write_literal("<html>");
            writer.write_literal("<form method=\"post\"><textarea name=\"question\"></textarea><textarea name=\"notes\"></textarea><input type=\"submit\"/></form>");
            writer.write_literal("</html>");

            ByteStringRef encoded = std::make_shared<ByteString>();
            utf_8_encode(response_body.get(), encoded.get());

            // $ set charset in mediatype

            response->server_body = encoded;
            response->headers->set_string(Http::globals->header_content_type, Basic::globals->text_html_media_type);
            response->code = 200;
            response->reason = Http::globals->reason_ok;
        }
        else if (equals<UnicodeString, true>(request->method.get(), Http::globals->post_method.get()))
        {
            UnicodeStringRef contentType;

            bool correct_content_type = false;

            bool success = request->headers->get_string(Http::globals->header_content_type, &contentType);
            if (success)
            {
                if (equals<UnicodeString, false>(contentType.get(), Http::globals->application_x_www_form_urlencoded_media_type.get()))
                    correct_content_type = true;
            }

            if (!correct_content_type)
            {
                response->code = 400;
                response->reason = Http::globals->reason_bad_request;
                return;
            }

            NameValueCollection formData;

            // $ replace with proper form handling
            //FormDataFrame frame;
            //frame.Initialize(&formData);

            //success = FrameStream<byte>::handle_event(&frame, (byte*)request->body->address(), request->body->size());
            //if (!success)
            //    return false;

            //HeadersFrame headersFrame;
            //headersFrame.Initialize(&formData);
            //headersFrame.write_to_stream(response_body);

            UnicodeStringRef response_body = std::make_shared<UnicodeString>();
            response_body->reserve(0x400);

            TextWriter writer(response_body.get());
            writer.write_literal("<html>");
            writer.write_literal("<form method=\"post\"><textarea name=\"question\"></textarea><textarea name=\"notes\"></textarea><input type=\"submit\"/></form>");
            writer.write_literal("<pre>");
            // $ text rendered form data goes here
            writer.write_literal("</pre></html>");

            ByteStringRef encoded = std::make_shared<ByteString>();
            utf_8_encode(response_body.get(), encoded.get());
            
            // $ set charset in mediatype

            response->server_body = encoded;
            response->headers->set_string(Http::globals->header_content_type, Basic::globals->text_html_media_type);
            response->code = 200;
            response->reason = Http::globals->reason_ok;
        }
        else
        {
            response->code = 405;
            response->reason = Http::globals->reason_method;
        }
    }
}