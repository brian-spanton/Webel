// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.ListenSocket.h"

namespace Basic
{
	void ListenSocket::Initialize(Face face, short port)
	{
		__super::Initialize();

		sockaddr_in endpoint;
		endpoint.sin_family = AF_INET;
		endpoint.sin_port = htons(port);

		switch (face)
		{
		case Face_Local:
			endpoint.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
			break;

		case Face_External:
			{
				hostent* host = gethostbyname(0);
				endpoint.sin_addr = *reinterpret_cast<in_addr*>(host->h_addr_list[0]);
			}
			break;

		case Face_Default:
			endpoint.sin_addr.S_un.S_addr = INADDR_ANY;
			break;
		}

		int error = bind(socket, reinterpret_cast<const sockaddr*>(&endpoint), sizeof(endpoint));
		if (error == SOCKET_ERROR)
			throw new Exception("bind", WSAGetLastError());

		error = listen(socket, SOMAXCONN);
		if (error == SOCKET_ERROR)
			throw new Exception("listen", WSAGetLastError());

		Basic::globals->DebugWriter()->WriteFormat<0x40>("listening on port %d", port);
		Basic::globals->DebugWriter()->WriteLine();
	}

	void ListenSocket::StartAccept(ServerSocket* acceptPeer)
	{
		AsyncBytes::Ref bytes = New<AsyncBytes>("5");
		bytes->Initialize(0x1000);
		bytes->PrepareForAccept("ListenSocket::StartAccept AcceptEx", acceptPeer);

		uint32 count;
		BOOL success2 = AcceptEx(
			this->socket,
			acceptPeer->socket,
			bytes->bytes,
			bytes->maxCount - addressLength * 2,
			addressLength,
			addressLength,
			&count,
			bytes);
		if (success2 == FALSE)
		{
			int error = WSAGetLastError();
			if (error != ERROR_IO_PENDING)
				throw new Exception("ListenSocket::StartAccept AcceptEx", error);
		}
	}

	void ListenSocket::CompleteRead(AsyncBytes* bytes, int transferred, int error)
	{
		if (error != ERROR_SUCCESS)
		{
			if (error != STATUS_CONNECTION_RESET && error != STATUS_CANCELLED)
				throw new Exception("ListenSocket::CompleteRead", error);
		}
		else
		{
			error = setsockopt(
				bytes->acceptPeer->socket,
				SOL_SOCKET,
				SO_UPDATE_ACCEPT_CONTEXT,
				reinterpret_cast<char*>(&this->socket),
				sizeof(this->socket));
			if (error == SOCKET_ERROR)
				throw new Exception("ListenSocket::CompleteRead setsockopt", WSAGetLastError());

			bytes->acceptPeer->CompleteAccept(bytes, transferred);
		}
	}
}