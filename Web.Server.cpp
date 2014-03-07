// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Web.Server.h"
#include "Basic.ServerSocket.h"
#include "Basic.FrameStream.h"
#include "Basic.DebugStream.h"
#include "Basic.CountStream.h"
#include "Web.Globals.h"
#include "Http.Globals.h"
#include "Http.ResponseHeadersFrame.h"
#include "Http.HeadersFrame.h"

namespace Web
{
	using namespace Basic;

	void Server::Initialize(ListenSocket* listen_socket, Basic::Ref<Tls::ICertificate> certificate, Basic::Ref<IProcess> completion, ByteString::Ref cookie)
	{
		__super::Initialize();

		this->accept_completion = completion;
		this->accept_cookie = cookie;

		ServerSocket::Ref server_socket;
		Web::globals->CreateServerSocket(certificate, this, &server_socket, &this->peer);

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
				Basic::globals->DebugWriter()->WriteLine("accepted");

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

				Basic::globals->DebugWriter()->Write("Request received: ");
				Basic::globals->DebugWriter()->Write((const char*)request_line.c_str(), request_line.size());
				Basic::globals->DebugWriter()->WriteLine();

				this->response = New<Response>();
				this->response->Initialize();

				Process();

				this->response->protocol = Http::globals->HTTP_1_1;

				if (this->response->server_body.item() != 0)
				{
					Inline<Basic::CountStream<byte> > count;

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

				Basic::globals->DebugWriter()->Write("Response sent: ");
				Basic::globals->DebugWriter()->Write((const char*)response_bytes.c_str(), response_bytes.size());
				Basic::globals->DebugWriter()->WriteLine();
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
			throw new Exception("Web::Server::Process unexpected state");
		}
	}
}