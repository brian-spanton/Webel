// Copyright © 2013 Brian Spanton

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

	public:
		typedef Basic::Ref<LogFile> Ref;

		HANDLE file;

		LogFile();
		virtual ~LogFile();

		void Initialize(const char* name);

		virtual void Write(AsyncBytes* line);
		virtual void ICompletion::CompleteAsync(OVERLAPPED_ENTRY& entry);
	};
}