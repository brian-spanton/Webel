// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.ICompletion.h"
#include "Basic.AsyncBytes.h"

namespace Basic
{
	class Socket : public ICompletion
	{
	protected:
		static const DWORD addressLength;

		Lock lock;

		void Initialize();
		virtual void CompleteRead(AsyncBytes* bytes, int transferred, int error);
		virtual void CompleteWrite(AsyncBytes* bytes, int transferred, int error);
		virtual void CompleteOther(int transferred, int error);

	public:
		typedef Basic::Ref<Socket, ICompletion> Ref;

		SOCKET socket;

		Socket();
		virtual ~Socket();

		virtual void ICompletion::CompleteAsync(OVERLAPPED_ENTRY& entry);
	};
}