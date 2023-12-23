#pragma once

#include "Basic.ConnectedSocket.h"

namespace Basic
{
	class ListenSocket;

	class ServerSocket : public ConnectedSocket
	{
	private:
		friend class ListenSocket;

		void CompleteAccept(AsyncBytes* bytes, uint32 count);

	public:
		typedef Basic::Ref<ServerSocket, ICompletion> Ref;

		void Initialize(IProcess* protocol, bool secure, Basic::Ref<IBufferedStream<byte> >* peer);
	};
}