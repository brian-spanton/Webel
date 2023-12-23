#pragma once

namespace Basic
{
	class _declspec(novtable) Lock
	{
	private:
		CRITICAL_SECTION lock;

	public:
		Lock()
		{
			InitializeCriticalSection(&lock);
		}

		void Acquire()
		{
			EnterCriticalSection(&lock);
		}

		void Release()
		{
			LeaveCriticalSection(&lock);
		}

		~Lock()
		{
			DeleteCriticalSection(&lock);
		}
	};
}