#pragma once

#include "Basic.ServerSocket.h"
#include "Basic.Hold.h"
#include "Basic.IProtocolFactory.h"

namespace Basic
{
	class ListenSocket : public Socket
	{
	protected:
		virtual void CompleteRead(AsyncBytes* bytes, int transferred, int error);

	public:
		typedef Basic::Ref<ListenSocket, ICompletion> Ref;

		enum Face
		{
			Face_Local,
			Face_External,
			Face_Default,
		};

		void Initialize(Face face, short port);
		void StartAccept(ServerSocket* acceptPeer);
	};
}