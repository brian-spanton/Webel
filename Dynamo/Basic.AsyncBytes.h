#pragma once

#include "Basic.TextWriter.h"
#include "Basic.RefCounted.h"
#include "Basic.IRefCounted.h"
#include "Basic.Ref.h"
#include "Basic.IStream.h"
#include "Basic.ICompletion.h"

namespace Basic
{
	class ServerSocket;

	class AsyncBytes : public OVERLAPPED, public IStream<byte>, public IRefHolder
	{
	public:
		typedef Basic::Ref<AsyncBytes> Ref;

		static AsyncBytes* FromOverlapped(OVERLAPPED* overlapped);

		Basic::Ref<IRefCounted> pending_io_ref;
		int maxCount;
		uint32 count;
		byte* bytes; // $ why can't this just be a ByteString::Ref or the like?
		WSABUF wsabuf;
		bool receive;
		Basic::Ref<ServerSocket, ICompletion> acceptPeer;
		Basic::Ref<ICompletion> ioPeer;

		AsyncBytes();
		virtual ~AsyncBytes();

		void Initialize(int maxCount);
		void PrepareForAccept(const char* name, ServerSocket* acceptPeer);
		void PrepareForReceive(const char* name, ICompletion* ioPeer);
		void PrepareForSend(const char* name, ICompletion* ioPeer);
		void IoCompleted();

		virtual void IStream<byte>::Write(const byte* elements, uint32 count);
		virtual void IStream<byte>::WriteEOF();
	};
}