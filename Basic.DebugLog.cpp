// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.DebugLog.h"
#include "Basic.Hold.h"
#include "Basic.AsyncBytes.h"

namespace Basic
{
	DebugLog::DebugLog() :
		first(0),
		count(0)
	{
	}

	void DebugLog::Write(AsyncBytes* bytes)
	{
		bytes->bytes[bytes->count] = 0;
		OutputDebugStringA((LPCSTR)bytes->bytes);
		printf((LPCSTR)bytes->bytes);

		{
			Hold hold(tailLock);

			int next = (first + this->count) % _countof(tail);

			if (this->count == _countof(tail))
				first = (first + 1) % _countof(tail);
			else
				this->count++;

			tail[next] = bytes;
		}

		__super::Write(bytes);
	}

	void DebugLog::WriteTo(TextWriter* text)
	{
		Hold hold(tailLock);

		for (uint32 i = 0; i < this->count; i++)
		{
			int next = (first + i) % _countof(tail);
			text->Write((const char*)tail[next]->bytes, tail[next]->count);
		}
	}
}