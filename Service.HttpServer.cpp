// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Service.HttpServer.h"
#include "Basic.ServerSocket.h"
#include "Basic.FrameStream.h"
#include "Basic.DebugStream.h"
#include "Basic.CountStream.h"
#include "Http.Globals.h"
#include "Http.ResponseHeadersFrame.h"
#include "Http.HeadersFrame.h"
#include "Html.ElementNode.h"
#include "Service.Globals.h"

namespace Service
{
    using namespace Basic;

    void HttpServer::Process()
    {
        if (this->request->resource->path.size() > 0)
        {
            UnicodeString::Ref resource = this->request->resource->path.at(0);

            if (resource.equals<false>(Service::globals->root_admin))
            {
                AdminRequest(this->request, this->response);
            }
            else if (resource.equals<false>(Service::globals->root_echo))
            {
                EchoRequest(this->request, this->response);
            }
            else if (resource.equals<false>(Service::globals->root_question))
            {
                QuestionRequest(this->request, this->response);
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

    class AdminRequest : public IRefCounted
    {
    private:
        Request::Ref request;

    public:
        typedef Basic::Ref<AdminRequest> Ref;

        void Initialize(Request* request)
        {
            this->request = request;
        }

        void invoke_protocol(IStream<Codepoint>* peer)
        {
            Service::globals->adminProtocol->set_peer(peer);

            Inline<FrameStream<Codepoint> > frame_stream;
            frame_stream.Initialize(Service::globals->adminProtocol);

            if (this->request->resource->query.item() != 0)
            {
                Inline<StringMap> data_set;
                Web::Form::url_decode(this->request->resource->query, (UnicodeString*)0, false, &data_set);
                frame_stream.Write(data_set.begin()->second->c_str(), data_set.begin()->second->size());
            }

            Codepoint cr = Http::globals->CR;
            frame_stream.Write(&cr, 1);
            frame_stream.WriteEOF();
        }
    };

    void HttpServer::AdminRequest(Request* request, Response* response)
    {
        if (request->method.equals<true>(Http::globals->get_method))
        {
            AdminRequest::Ref admin_request = New<Service::AdminRequest>();
            admin_request->Initialize(request);

            // $ should send it back chunked instead of buffering
            UnicodeString::Ref response_body = New<UnicodeString>();
            admin_request->invoke_protocol(response_body);

            ByteString::Ref encoded = New<ByteString>();
            response_body->utf_8_encode(encoded);

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
        if (request->method.equals<true>(Http::globals->get_method))
        {
            ByteString::Ref body = New<ByteString>();
            body->reserve(0x400);

            Inline<RequestFrame> frame;
            frame.Initialize(request);
            frame.SerializeTo(body);

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
        if (request->method.equals<true>(Http::globals->get_method))
        {
            UnicodeString::Ref response_body = New<UnicodeString>();
            response_body->reserve(0x400);

            TextWriter writer(response_body);
            writer.Write("<html>");
            writer.Write("<form method=\"post\"><textarea name=\"question\"></textarea><textarea name=\"notes\"></textarea><input type=\"submit\"/></form>");
            writer.Write("</html>");

            ByteString::Ref encoded = New<ByteString>();
            response_body->utf_8_encode(encoded);

            // $ set charset in mediatype

            response->server_body = encoded;
            response->headers->set_string(Http::globals->header_content_type, Basic::globals->text_html_media_type);
            response->code = 200;
            response->reason = Http::globals->reason_ok;
        }
        else if (request->method.equals<true>(Http::globals->post_method))
        {
            UnicodeString::Ref contentType;

            bool correct_content_type = false;

            bool success = request->headers->get_string(Http::globals->header_content_type, &contentType);
            if (success)
            {
                if (contentType.equals<false>(Http::globals->application_x_www_form_urlencoded_media_type))
                    correct_content_type = true;
            }

            if (!correct_content_type)
            {
                response->code = 400;
                response->reason = Http::globals->reason_bad_request;
                return;
            }

            Inline<NameValueCollection > formData;

            // $ replace with proper form handling
            //Inline<FormDataFrame> frame;
            //frame.Initialize(&formData);

            //success = FrameStream<byte>::Process(&frame, (byte*)request->body->c_str(), request->body->size());
            //if (!success)
            //    return false;

            //Inline<HeadersFrame> headersFrame;
            //headersFrame.Initialize(&formData);
            //headersFrame.SerializeTo(response_body);

            UnicodeString::Ref response_body = New<UnicodeString>();
            response_body->reserve(0x400);

            TextWriter writer(response_body);
            writer.Write("<html>");
            writer.Write("<form method=\"post\"><textarea name=\"question\"></textarea><textarea name=\"notes\"></textarea><input type=\"submit\"/></form>");
            writer.Write("<pre>");
            // $ text rendered form data goes here
            writer.Write("</pre></html>");

            ByteString::Ref encoded = New<ByteString>();
            response_body->utf_8_encode(encoded);
            
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