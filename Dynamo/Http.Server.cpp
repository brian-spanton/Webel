#include "stdafx.h"
#include "Http.Server.h"
#include "Basic.ServerSocket.h"
#include "Basic.FrameStream.h"
#include "Dynamo.AdminProtocol.h"
#include "Dynamo.Globals.h"
#include "Basic.DebugStream.h"
#include "Http.Globals.h"
#include "Http.ResponseHeadersFrame.h"
#include "Http.HeadersFrame.h"
#include "Tls.RecordLayer.h"
#include "Html.ElementNode.h"

namespace Http
{
	using namespace Basic;

	void Server::Initialize(ListenSocket* listen_socket, bool secure, Basic::Ref<IProcess> completion, ByteString::Ref cookie)
	{
		__super::Initialize();

		this->accept_completion = completion;
		this->accept_cookie = cookie;

		ServerSocket::Ref server_socket = New<ServerSocket>();
		server_socket->Initialize(this, secure, &this->peer);

		listen_socket->StartAccept(server_socket);
	}

	void Server::switch_to_state(State state)
	{
		__super::switch_to_state(state);

		if (!Pending())
			this->peer->WriteEOF();
	}

	void Server::Process(IEvent* event, bool* yield)
	{
		(*yield) = false;

		switch (frame_state())
		{
		case State::pending_connection_state:
			if (event->get_type() == Basic::EventType::ready_for_write_bytes_event)
			{
				Dynamo::globals->DebugWriter()->WriteLine("accepted");

				Basic::Ref<IProcess> completion = this->accept_completion;
				this->accept_completion = 0;

				AcceptCompleteEvent event;
				event.cookie = this->accept_cookie;
				this->accept_cookie = 0;

				if (completion.item() != 0)
					completion->Process(&event);

				switch_to_state(State::new_request_state);
				(*yield) = false;
			}
			else
			{
				(*yield) = true;
			}
			break;

		case State::new_request_state:
			{
				this->request = New<Request>();
				this->request->Initialize();

				this->request_frame.Initialize(this->request);

				switch_to_state(State::request_frame_pending_state);
				(*yield) = false;
			}
			break;

		case State::request_frame_pending_state:
			if (this->request_frame.Pending())
			{
				this->request_frame.Process(event, yield);
			}

			if (this->request_frame.Failed())
			{
				switch_to_state(State::request_frame_failed);
				(*yield) = false;
			}
			else if (this->request_frame.Succeeded())
			{
				Inline<ByteString> request_line;
				this->request_frame.WriteRequestLineTo(&request_line);

				Dynamo::globals->DebugWriter()->Write("Request received: ");
				Dynamo::globals->DebugWriter()->Write((const char*)request_line.c_str(), request_line.size());
				Dynamo::globals->DebugWriter()->WriteLine();

				this->response = New<Response>();
				this->response->Initialize();

				if (this->request->resource->path.size() > 0)
				{
					UnicodeString::Ref resource = this->request->resource->path.at(0);

					if (resource.equals<false>(Dynamo::globals->root_admin))
					{
						AdminRequest(this->request, this->response);
						// $ handle result, or better find a way to handle this dynamo specific stuff outside of Http::Server
					}
					else if (resource.equals<false>(Dynamo::globals->root_echo))
					{
						bool success = EchoRequest(this->request, this->response);
						// $ handle result, or better find a way to handle this dynamo specific stuff outside of Http::Server
					}
					else if (resource.equals<false>(Dynamo::globals->root_question))
					{
						bool success = QuestionRequest(this->request, this->response);
						// $ handle result, or better find a way to handle this dynamo specific stuff outside of Http::Server
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

				this->response->protocol = Http::globals->HTTP_1_1;

				if (this->response->server_body.item() != 0)
				{
					Inline<CountStream<byte> > count;

					this->response->server_body->SerializeTo(&count);

					this->response->headers->set_base_10(Http::globals->header_content_length, count.count);
				}
				else
				{
					this->response->headers->set_base_10(Http::globals->header_content_length, 0);
				}

				Inline<ResponseHeadersFrame> responseFrame;
				responseFrame.Initialize(this->request->method, this->response);

				responseFrame.SerializeTo(this->peer);
				this->peer->Flush();

				switch_to_state(State::response_done_state);
				(*yield) = false;

				Inline<ByteString> response_bytes;
				responseFrame.SerializeTo(&response_bytes);

				Dynamo::globals->DebugWriter()->Write("Response sent: ");
				Dynamo::globals->DebugWriter()->Write((const char*)response_bytes.c_str(), response_bytes.size());
				Dynamo::globals->DebugWriter()->WriteLine();
			}
			break;

		case State::response_done_state:
			{
				UnicodeString::Ref connection;
				bool success = this->request->headers->get_string(Http::globals->header_connection, &connection);
				if (success)
				{
					if (connection.equals<false>(Http::globals->keep_alive))
					{
						switch_to_state(State::new_request_state);
						(*yield) = false;
						return;
					}
				}

				switch_to_state(State::done_state);
			}
			break;

		default:
			throw new Exception("Http::Server::Process unexpected state");
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
			Dynamo::globals->adminProtocol->set_peer(peer);

			Inline<FrameStream<Codepoint> > frame_stream;
			frame_stream.Initialize(Dynamo::globals->adminProtocol);

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

	void Server::AdminRequest(Request* request, Response* response)
	{
		if (request->method.equals<true>(Http::globals->get_method))
		{
			AdminRequest::Ref admin_request = New<Http::AdminRequest>();
			admin_request->Initialize(request);

			// $$ should send it back chunked instead of buffering
			UnicodeString::Ref response_body = New<UnicodeString>();
			admin_request->invoke_protocol(response_body);

			ByteString::Ref encoded = New<ByteString>();
			response_body->utf_8_encode(encoded);

			// $$ set charset in mediatype

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

	bool Server::EchoRequest(Request* request, Response* response)
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

			return true;
		}

		response->code = 405;
		response->reason = Http::globals->reason_method;
		return true;
	}

	bool Server::QuestionRequest(Request* request, Response* response)
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

			return true;
		}
		else if (request->method.equals<true>(Http::globals->post_method))
		{
			UnicodeString::Ref contentType;

			bool success = request->headers->get_string(Http::globals->header_content_type, &contentType);
			if (!success)
				return false;

			if (!contentType.equals<false>(Http::globals->application_x_www_form_urlencoded_media_type))
				return Basic::globals->HandleError("QuestionRequest::GetBodyStream contentType != application/x-www-form-urlencoded", 0);

			Inline<NameValueCollection > formData;

			// $ replace with proper form handling
			//Inline<FormDataFrame> frame;
			//frame.Initialize(&formData);

			//success = FrameStream<byte>::Process(&frame, (byte*)request->body->c_str(), request->body->size());
			//if (!success)
			//	return false;

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

			return true;
		}

		response->code = 405;
		response->reason = Http::globals->reason_method;
		return true;
	}
}