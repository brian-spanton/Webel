#pragma once

#include "Basic.LogFile.h"
#include "Basic.Console.h"
#include "Basic.AsyncBytes.h"
#include "Basic.Lock.h"

namespace Basic
{
	class DebugLog : public LogFile
	{
	private:
		Lock tailLock;
		AsyncBytes::Ref tail[0x400]; // $$$
		int first;
		uint32 count;

	public:
		typedef Basic::Ref<DebugLog, LogFile> Ref;

		DebugLog();

		virtual void Write(AsyncBytes* bytes);
		virtual void WriteTo(TextWriter* text);
	};
}