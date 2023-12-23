#pragma once

#include "Basic.Lock.h"
#include "Basic.IBufferedStream.h"
#include "Basic.IProcess.h"
#include "Basic.ElementSource.h"

namespace Basic
{
	class Console : public IBufferedStream<Codepoint>
	{
	private:
		Lock lock;
		HANDLE output;
		HANDLE input;
		DWORD originalMode;
		Ref<IProcess> protocol; // $$$
		Inline<ElementSource<Codepoint> > protocol_element_source;

		static DWORD WINAPI Thread(void* param);
		bool TryInitialize(IProcess* protocol, HANDLE* createdThread);

	public:
		Console();
		virtual ~Console();

		void Initialize(IProcess* protocol, HANDLE* createdThread);
		bool Thread();

		virtual void IBufferedStream<Codepoint>::Write(const Codepoint* elements, uint32 count);
		virtual void IBufferedStream<Codepoint>::Flush();
		virtual void IBufferedStream<Codepoint>::WriteEOF();
	};
}