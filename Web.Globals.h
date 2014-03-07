// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ClientSocket.h"
#include "Basic.ServerSocket.h"
#include "Tls.ICertificate.h"

namespace Web
{
	using namespace Basic;

	class Globals
	{
	public:
		Globals();

		void Initialize();

		void CreateServerSocket(Basic::Ref<Tls::ICertificate> certificate, IProcess* protocol, Basic::ServerSocket::Ref* socket, Basic::Ref<IBufferedStream<byte> >* peer);
		void CreateClientSocket(bool secure, IProcess* protocol, Basic::ClientSocket::Ref* socket, Basic::Ref<IBufferedStream<byte> >* peer);
	};

	extern Globals* globals;
}
