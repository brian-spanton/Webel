#pragma once

#include "Basic.ICompletion.h"
#include "Basic.IStream.h"
#include "Basic.AsyncBytes.h"

namespace Basic
{
	class LogFile : public ICompletion
	{
	private:
		static byte encoding[];

		HANDLE file;

	public:
		LogFile();
		virtual ~LogFile();

		void Initialize(const char* name, HANDLE queue);

		virtual void Write(AsyncBytes* line);
		virtual void ICompletion::CompleteAsync(OVERLAPPED_ENTRY& entry);
	};
}