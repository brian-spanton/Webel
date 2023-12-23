#include "stdafx.h"
#include "Basic.ConnectedSocket.h"
#include "Basic.AsyncBytes.h"
#include "Basic.Globals.h"
#include "Tls.RecordLayer.h"
#include "Basic.Hold.h"
#include "Dynamo.Globals.h"

namespace Basic
{
	ConnectedSocket::~ConnectedSocket()
	{
	}

	void ConnectedSocket::Initialize(IProcess* protocol, bool secure, bool server, Basic::Ref<IBufferedStream<byte> >* peer)
	{
		__super::InitializeSocket();

		this->secure = secure;

		if (secure)
		{
			Tls::RecordLayer::Ref tls_frame = New<Tls::RecordLayer>();
			tls_frame->Initialize(this, protocol, server);

			this->protocol = tls_frame;
			(*peer) = tls_frame;
		}
		else
		{
			this->protocol = protocol;
			(*peer) = this;
		}
	}

	void ConnectedSocket::InitializePeer(sockaddr_in* remoteAddress)
	{
		this->remoteAddress = (*remoteAddress);

		UnicodeString::Ref id = New<UnicodeString>();
		id->reserve(0x40);

		TextWriter text(id);
		text.WriteFormat<0x40>(
			"%d.%d.%d.%d:%d",
			this->remoteAddress.sin_addr.S_un.S_un_b.s_b1,
			this->remoteAddress.sin_addr.S_un.S_un_b.s_b2,
			this->remoteAddress.sin_addr.S_un.S_un_b.s_b3,
			this->remoteAddress.sin_addr.S_un.S_un_b.s_b4,
			this->remoteAddress.sin_port);
	}

	void ConnectedSocket::Received(const byte* bytes, uint32 count)
	{
		if (socket != INVALID_SOCKET)
		{
			if (count == 0)
			{
				DisconnectAndNotifyProtocol();
			}
			else
			{
				this->protocol_element_source.Initialize(bytes, count);

				ReadyForReadBytesEvent event;
				event.Initialize(&this->protocol_element_source);

				this->protocol->Process(&event);

				if (!this->protocol_element_source.Exhausted() && !this->protocol->Failed())
					throw new Exception("Basic::ReadyForReadBytesEvent::Consume this->elements_read < this->count");
			}
		}
	}

	void ConnectedSocket::StartReceive()
	{
		uint32 count;
		DWORD flags = 0;

		AsyncBytes::Ref bytes = New<AsyncBytes>("2");
		bytes->Initialize(0x400);
		bytes->PrepareForReceive("ConnectedSocket::StartReceive WSARecv", this);

		int error = WSARecv(socket, &bytes->wsabuf, 1, &count, &flags, bytes, 0);
		if (error == SOCKET_ERROR)
		{
			error = WSAGetLastError();
			if (error != ERROR_IO_PENDING)
			{
				bytes->Internal = error;
				Dynamo::globals->PostCompletion(this, bytes);
			}
		}
	}

	void ConnectedSocket::DisconnectAndNotifyProtocol()
	{
		Ref<IProcess> protocol;
		Disconnect(&protocol);

		if (protocol.item() != 0)
		{
			ElementStreamEndingEvent event;
			protocol->Process(&event);
		}
	}

	void ConnectedSocket::Disconnect(Ref<IProcess>* protocol)
	{
		Hold hold(this->lock);

		if (socket != INVALID_SOCKET)
		{
			shutdown(socket, SD_BOTH);
			closesocket(socket);

			socket = INVALID_SOCKET;
		}

		if (protocol != 0)
			(*protocol) = this->protocol;

		this->protocol = 0;
	}

	void ConnectedSocket::Write(const byte* elements, uint32 count)
	{
		while(true)
		{
			if (count == 0)
				break;

			AsyncBytes::Ref buffer = New<AsyncBytes>("3");
			buffer->Initialize(0x1000);

			uint32 remaining = buffer->maxCount - buffer->count;
			uint32 useable = (count > remaining) ? remaining : count;

			buffer->Write(elements, useable);

			count -= useable;
			elements += useable;

			Send(buffer);
		}
	}

	void ConnectedSocket::Send(AsyncBytes* buffer)
	{
		if (buffer->count == 0)
			throw new Exception("ConnectedSocket::Send 0 bytes");

		uint32 count;
		DWORD flags = 0;

		buffer->PrepareForSend("ConnectedSocket::Flush WSASend", this);

		int error = WSASend(socket, &buffer->wsabuf, 1, &count, flags, buffer, 0);
		if (error == SOCKET_ERROR)
		{
			error = WSAGetLastError();
			if (error != ERROR_IO_PENDING)
			{
				buffer->Internal = error;
				Dynamo::globals->PostCompletion(this, buffer);
			}
		}
	}

	void ConnectedSocket::Flush()
	{
	}

	void ConnectedSocket::WriteEOF()
	{
		Disconnect(0);
	}

	void ConnectedSocket::CompleteRead(AsyncBytes* bytes, int transferred, int error)
	{
		if (error != ERROR_SUCCESS && error != STATUS_PENDING)
		{
			if (error != STATUS_CONNECTION_RESET && error != STATUS_CONNECTION_ABORTED && error != STATUS_CANCELLED)
				Basic::globals->HandleError("ConnectedSocket::CompleteRead", error);

			DisconnectAndNotifyProtocol();
		}
		else
		{
			Received(bytes->bytes, transferred);

			if (socket != INVALID_SOCKET)
				StartReceive();
		}
	}
}