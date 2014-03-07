// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Web.Proxy.h"
#include "Basic.ServerSocket.h"
#include "Basic.FrameStream.h"
#include "Basic.DebugStream.h"
#include "Web.Globals.h"
#include "Http.Globals.h"
#include "Http.ResponseHeadersFrame.h"
#include "Http.HeadersFrame.h"
#include "Basic.ClientSocket.h"

namespace Web
{
	using namespace Basic;

	void Proxy::Initialize(ListenSocket* listen_socket, Basic::Ref<Tls::ICertificate> certificate, Basic::Ref<IProcess> completion, ByteString::Ref cookie, Uri::Ref server_url)
	{
		__super::Initialize();

		this->server_url = server_url;
		this->accept_completion = completion;
		this->accept_cookie = cookie;

		ServerSocket::Ref server_socket;
		Web::globals->CreateServerSocket(certificate, this, &server_socket, &this->client_peer);

		listen_socket->StartAccept(server_socket);
	}

	void Proxy::switch_to_state(State state)
	{
		__super::switch_to_state(state);

		if (!Pending())
		{
			if (this->client_peer.item() != 0)
				this->client_peer->WriteEOF();

			if (this->server_peer.item() != 0)
				this->server_peer->WriteEOF();
		}
	}

	void Proxy::Process(IEvent* event, bool* yield)
	{
		Hold hold(this->lock);

		(*yield) = true;

		switch (frame_state())
		{
		case State::pending_client_connection_state:
			switch (event->get_type())
			{
			case Basic::EventType::element_stream_ending_event:
				switch_to_state(State::done_state);
				break;

			case Basic::EventType::ready_for_write_bytes_event:
				{
					Basic::globals->DebugWriter()->WriteLine("accepted");

					Basic::Ref<IProcess> completion = this->accept_completion;
					this->accept_completion = 0;

					AcceptCompleteEvent event;
					event.cookie = this->accept_cookie;
					this->accept_cookie = 0;

					if (completion.item() != 0)
						completion->Process(&event);

					this->buffer = New<ByteVector>();

					ServerProxy::Ref server_proxy = New<ServerProxy>();
					server_proxy->Initialize(this);

					ClientSocket::Ref client_socket;
					Web::globals->CreateClientSocket(this->server_url->is_secure_scheme(), server_proxy, &client_socket, &this->server_peer);

					sockaddr_in addr;
					bool success = client_socket->Resolve(this->server_url->host, this->server_url->get_port(), &addr);
					if (!success)
					{
						HandleError("resolve failed");
						switch_to_state(State::done_state);
						return;
					}

					client_socket->StartConnect(addr);

					switch_to_state(State::pending_server_connection_state);
				}
				break;

			default:
				throw new Exception("unexpected event");
			}
			break;

		case State::pending_server_connection_state:
			switch (event->get_type())
			{
			case Basic::EventType::element_stream_ending_event:
				switch_to_state(State::done_state);
				break;

			case Basic::EventType::ready_for_read_bytes_event:
				{
					ReadyForReadBytesEvent* read_event = (ReadyForReadBytesEvent*)event;

					const byte* bytes;
					uint32 count;

					read_event->element_source->Read(0xffffffff, &bytes, &count, yield);

					if (count > 0)
						this->buffer->Write(bytes, count);
				}
				break;

			default:
				throw new Exception("unexpected event");
			}
			break;

		case State::connected_state:
			switch (event->get_type())
			{
			case Basic::EventType::element_stream_ending_event:
				switch_to_state(State::done_state);
				break;

			case Basic::EventType::ready_for_read_bytes_event:
				{
					ReadyForReadBytesEvent* read_event = (ReadyForReadBytesEvent*)event;

					const byte* bytes;
					uint32 count;

					read_event->element_source->Read(0xffffffff, &bytes, &count, yield);

					if (count > 0)
					{
						this->server_peer->Write(bytes, count);
						this->server_peer->Flush();
					}
				}
				break;

			default:
				throw new Exception("unexpected event");
			}
			break;

		default:
			throw new Exception("Web::Proxy::Process unexpected state");
		}
	}

	void Proxy::process_from_server(IEvent* event, bool* yield)
	{
		Hold hold(this->lock);

		(*yield) = true;

		switch (frame_state())
		{
		case State::pending_server_connection_state:
			switch (event->get_type())
			{
			case Basic::EventType::element_stream_ending_event:
				switch_to_state(State::done_state);
				break;

			case Basic::EventType::ready_for_write_bytes_event:
				if (this->buffer->size() > 0)
				{
					this->buffer->SerializeTo(this->server_peer);
					this->buffer->clear();
					this->server_peer->Flush();
				}

				switch_to_state(State::connected_state);
				break;

			default:
				throw new Exception("unexpected event");
			}
			break;

		case State::connected_state:
			switch (event->get_type())
			{
			case Basic::EventType::element_stream_ending_event:
				switch_to_state(State::done_state);
				break;

			case Basic::EventType::ready_for_read_bytes_event:
				{
					ReadyForReadBytesEvent* read_event = (ReadyForReadBytesEvent*)event;

					const byte* bytes;
					uint32 count;

					read_event->element_source->Read(0xffffffff, &bytes, &count, yield);

					if (count > 0)
					{
						this->client_peer->Write(bytes, count);
						this->client_peer->Flush();
					}
				}
				break;

			default:
				throw new Exception("unexpected event");
			}
			break;

		default:
			throw new Exception("Web::Proxy::Process unexpected state");
		}
	}

	void ServerProxy::Initialize(Proxy::Ref proxy)
	{
		__super::Initialize();

		this->proxy = proxy;
	}

	void ServerProxy::Process(IEvent* event, bool* yield)
	{
		this->proxy->process_from_server(event, yield);
	}
}