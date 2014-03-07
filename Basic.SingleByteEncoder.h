// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IEncoder.h"
#include "Basic.IEncoderFactory.h"
#include "Basic.ISingleByteEncodingIndex.h"

namespace Basic
{
	class SingleByteEncoder : public IEncoder
	{
	private:
		Ref<IStream<byte> > destination; // REF
		Ref<ISingleByteEncodingIndex> index; // REF
		byte error_replacement_byte;

		void Emit(byte b);
		void EncoderError(Codepoint codepoint);

	public:
		typedef Basic::Ref<SingleByteEncoder> Ref;

		SingleByteEncoder();

		void Initialize(ISingleByteEncodingIndex* index);
		void Initialize(ISingleByteEncodingIndex* index, IStream<byte>* destination);

		void IEncoder::set_destination(IStream<byte>* destination);
		void IEncoder::set_error_replacement_byte(byte b);
		void IEncoder::Write(const Codepoint* elements, uint32 count);
		void IEncoder::WriteEOF();
	};

	class SingleByteEncoderFactory : public IEncoderFactory
	{
	private:
		Ref<ISingleByteEncodingIndex> index; // REF

	public:
		typedef Ref<SingleByteEncoderFactory, IEncoderFactory> Ref;

		void Initialize(ISingleByteEncodingIndex* index);

		virtual void IEncoderFactory::CreateEncoder(Basic::Ref<IEncoder>* encoder);
	};
}