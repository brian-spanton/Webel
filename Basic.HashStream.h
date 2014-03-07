// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.Ref.h"
#include "Basic.HashAlgorithm.h"

namespace Basic
{
	class HashStream : public IStream<byte>
	{
	private:
		byte hash_object[0x1000];
		BCRYPT_HASH_HANDLE hash;
		byte* output;
		uint32 output_length;

	public:
		typedef Basic::Ref<HashStream, IStream<byte> > Ref;

		HashStream();
		virtual ~HashStream();

		void Initialize(HashAlgorithm* algorithm, byte* secret, uint32 secret_length, byte* output, uint32 output_max);

		virtual void IStream<byte>::Write(const byte* elements, uint32 count);
		virtual void IStream<byte>::WriteEOF();
	};
}