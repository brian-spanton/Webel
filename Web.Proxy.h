// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ListenSocket.h"
#include "Basic.ByteVector.h"
#include "Http.RequestFrame.h"
#include "Tls.ICertificate.h"

namespace Web
{
	using namespace Basic;
	using namespace Http;

	class Proxy : public Frame
	{
	private:
		enum State
		{
			pending_client_connection_state = Start_State,
			pending_server_connection_state,
			connected_state,
			done_state = Succeeded_State,
		};

		Basic::Ref<IBufferedStream<byte> > client_peer; // REF
		Basic::Ref<IBufferedStream<byte> > server_peer; // REF
		Basic::Ref<IProcess> accept_completion; // REF
		ByteString::Ref accept_cookie; // REF
		Uri::Ref server_url; // REF
		Lock lock;
		Basic::ByteVector::Ref buffer; // REF

		void switch_to_state(State state);

	public:
		typedef Basic::Ref<Proxy, IProcess> Ref;

		void Initialize(ListenSocket* listen_socket, Basic::Ref<Tls::ICertificate> certificate, Basic::Ref<IProcess> completion, ByteString::Ref cookie, Uri::Ref server_url);

		virtual void IProcess::Process(IEvent* event, bool* yield);
		void process_from_server(IEvent* event, bool* yield);
	};

	class ServerProxy : public Frame
	{
	private:
		Proxy::Ref proxy; // REF

	public:
		typedef Basic::Ref<ServerProxy, IProcess> Ref;

		void Initialize(Proxy::Ref proxy);

		virtual void IProcess::Process(IEvent* event, bool* yield);
	};
}