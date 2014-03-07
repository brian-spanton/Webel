// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.Ref.h"

namespace Http
{
	using namespace Basic;

	class UrlDecoder : public IStream<byte>
	{
	private:
		enum State
		{
			normal_state,
			hex1_state,
			hex2_state,
			hex1_error,
			hex2_error,
		};

		Ref<IStream<byte> > destination; // REF
		byte hex;

	public:
		typedef Basic::Ref<UrlDecoder> Ref;

		State state;

		void Initialize(IStream<byte>* destination);

		virtual void IStream<byte>::Write(const byte* elements, uint32 count);
		virtual void IStream<byte>::WriteEOF();
	};
}