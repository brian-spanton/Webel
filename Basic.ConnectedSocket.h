// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Socket.h"
#include "Basic.IStream.h"
#include "Basic.IProcess.h"
#include "Basic.ElementSource.h"
#include "Basic.Lock.h"
#include "Basic.IBufferedStream.h"

namespace Basic
{
	class ConnectedSocket : public Socket, public IBufferedStream<byte>
	{
	protected:
		Basic::Ref<IProcess> protocol; // REF
		Inline<ElementSource<byte> > protocol_element_source;
		sockaddr_in remoteAddress;

		void StartReceive();
		void Received(const byte* bytes, uint32 count);
		void InitializePeer(sockaddr_in* remoteAddress);
		void Send(AsyncBytes* buffer);
		void Disconnect(Basic::Ref<IProcess>* protocol);
		void DisconnectAndNotifyProtocol();
		virtual void CompleteRead(AsyncBytes* bytes, int transferred, int error);

	public:
		typedef Basic::Ref<ConnectedSocket, IStream<byte> > Ref;

		virtual ~ConnectedSocket();
		void InitializeProtocol(IProcess* protocol);

		virtual void IBufferedStream<byte>::Write(const byte* elements, uint32 count);
		virtual void IBufferedStream<byte>::Flush();
		virtual void IBufferedStream<byte>::WriteEOF();
	};
}