// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.AsyncBytes.h"
#include "Basic.ServerSocket.h"
#include "Basic.Globals.h"

namespace Basic
{
	AsyncBytes* AsyncBytes::FromOverlapped(OVERLAPPED* overlapped)
	{
		return (AsyncBytes*)overlapped;
	}

	AsyncBytes::AsyncBytes() :
		bytes(0),
		maxCount(0),
		count(0)
	{
		this->pending_io_ref.SetHolder(this);
		this->acceptPeer.SetHolder(this);
		this->ioPeer.SetHolder(this);

		this->hEvent = 0;
		this->Internal = 0;
		this->InternalHigh = 0;
		this->Offset = 0;
		this->OffsetHigh = 0;

		this->wsabuf.buf = 0;
		this->wsabuf.len = 0;
	}

	AsyncBytes::~AsyncBytes()
	{
		if (this->bytes != 0)
			delete [] bytes;
	}

	void AsyncBytes::Initialize(int maxCount)
	{
		this->bytes = new byte[maxCount];
		this->maxCount = maxCount;
		this->wsabuf.buf = reinterpret_cast<CHAR*>(this->bytes);
	}

	void AsyncBytes::PrepareForAccept(const char* name, ServerSocket* acceptPeer)
	{
		this->acceptPeer = acceptPeer;
		PrepareForReceive(name, acceptPeer);
	}

	void AsyncBytes::PrepareForReceive(const char* name, ICompletion* ioPeer)
	{
		this->ioPeer = ioPeer;
		this->wsabuf.len = maxCount;
		this->receive = true;
		this->pending_io_ref = this;
	}

	void AsyncBytes::PrepareForSend(const char* name, ICompletion* ioPeer)
	{
		this->ioPeer = ioPeer;
		this->wsabuf.len = this->count;
		this->receive = false;
		this->pending_io_ref = this;
	}

	void AsyncBytes::IoCompleted()
	{
		this->pending_io_ref = 0;
	}

	void AsyncBytes::Write(const byte* elements, uint32 count)
	{
		uint32 remaining = this->maxCount - this->count;
		if (count > remaining)
			throw new Exception("AsyncBytes::Write count > remaining");

		CopyMemory(this->bytes + this->count, elements, count);
		this->count += count;
	}

	void AsyncBytes::WriteEOF()
	{
	}
}