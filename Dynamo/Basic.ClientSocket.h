#pragma once

#include "Basic.ConnectedSocket.h"

namespace Basic
{
	class ClientSocket : public ConnectedSocket, public IRefHolder
	{
	private:
		enum State
		{
			new_state,
			bound_state,
			connecting_state,
			receiving_state,
		};

		State state;
		OVERLAPPED resolveAddress;
		AsyncBytes::Ref sendBuffer;

		virtual void CompleteOther(int transferred, int error);
		virtual void CompleteWrite(AsyncBytes* bytes, int transferred, int error);

	public:
		typedef Basic::Ref<ClientSocket, IStream<byte> > Ref;

		ClientSocket();

		bool Resolve(UnicodeString::Ref host, uint16 port, sockaddr_in* remoteAddress);
		void Initialize(IProcess* protocol, bool secure, Basic::Ref<IBufferedStream<byte> >* peer);
		void StartConnect(IN_ADDR server, uint16 port);
		void StartConnect(sockaddr_in remoteAddress);

		virtual void IBufferedStream<byte>::Write(const byte* elements, uint32 count);
	};
}