#include "stdafx.h"
#include "Basic.ServerSocket.h"
#include "Basic.Event.h"
#include "Basic.ListenSocket.h"
#include "Dynamo.Globals.h"

namespace Basic
{
	void ServerSocket::Initialize(IProcess* protocol, bool secure, Basic::Ref<IBufferedStream<byte> >* peer)
	{
		__super::Initialize(protocol, secure, true, peer);
	}

	void ServerSocket::CompleteAccept(AsyncBytes* bytes, uint32 count)
	{
		sockaddr* localAddress;
		int localLength;

		sockaddr* remoteAddress;
		int remoteLength;

		GetAcceptExSockaddrs(
			bytes->bytes,
			bytes->maxCount - addressLength * 2,
			addressLength,
			addressLength,
			&localAddress,
			&localLength,
			&remoteAddress,
			&remoteLength);

		InitializePeer((sockaddr_in*)&remoteAddress);

		ReadyForWriteBytesEvent event;
		event.Initialize(&this->protocol_element_source);
		this->protocol->Process(&event);

		if (count > 0)
			Received(bytes->bytes, count);

		if (socket != INVALID_SOCKET)
			StartReceive();
	}
}