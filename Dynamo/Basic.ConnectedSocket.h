#pragma once

#include "Basic.Socket.h"
#include "Basic.IStream.h"
#include "Basic.ElementSource.h"
#include "Basic.Lock.h"
#include "Basic.IBufferedStream.h"

namespace Basic
{
	class ConnectedSocket : public Socket, public IBufferedStream<byte>
	{
	protected:
		Ref<IProcess> protocol; // $$$
		Inline<ElementSource<byte> > protocol_element_source;
		sockaddr_in remoteAddress;
		bool secure;

		void StartReceive();
		void Received(const byte* bytes, uint32 count);
		void InitializePeer(sockaddr_in* remoteAddress);
		void Send(AsyncBytes* buffer);
		void Disconnect(Ref<IProcess>* protocol);
		void DisconnectAndNotifyProtocol();
		virtual void CompleteRead(AsyncBytes* bytes, int transferred, int error);

	public:
		virtual ~ConnectedSocket();

		void Initialize(IProcess* protocol, bool secure, bool server, Basic::Ref<IBufferedStream<byte> >* peer);

		virtual void IBufferedStream<byte>::Write(const byte* elements, uint32 count);
		virtual void IBufferedStream<byte>::Flush();
		virtual void IBufferedStream<byte>::WriteEOF();
	};
}