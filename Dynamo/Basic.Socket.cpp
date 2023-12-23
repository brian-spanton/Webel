#include "stdafx.h"
#include "Basic.Socket.h"
#include "Dynamo.Globals.h"

namespace Basic
{
	const DWORD Socket::addressLength = sizeof(sockaddr_in) + 16;

	Socket::Socket() :
		socket(INVALID_SOCKET)
	{
	}

	Socket::~Socket()
	{
		if (socket != INVALID_SOCKET)
			closesocket(socket);
	}

	void Socket::InitializeSocket()
	{
		Dynamo::globals->CreateSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, this, &socket);
	}

	void Socket::CompleteAsync(OVERLAPPED_ENTRY& entry)
	{
		Hold hold(this->lock);

		int transferred = entry.dwNumberOfBytesTransferred;
		int error = ERROR_SUCCESS;

		if (entry.lpOverlapped != 0)
		{
			AsyncBytes::Ref bytes = AsyncBytes::FromOverlapped(entry.lpOverlapped);
			error = static_cast<int>(entry.lpOverlapped->Internal);

			bytes->IoCompleted();

			if (bytes->receive)
			{
				CompleteRead(bytes, transferred, error);
			}
			else
			{
				CompleteWrite(bytes, transferred, error);
			}
		}
		else
		{
			CompleteOther(transferred, error);
		}
	}

	void Socket::CompleteRead(AsyncBytes* bytes, int transferred, int error)
	{
	}

	void Socket::CompleteWrite(AsyncBytes* bytes, int transferred, int error)
	{
	}

	void Socket::CompleteOther(int transferred, int error)
	{
	}
}