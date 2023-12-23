#pragma once

#include "Basic.IDecoder.h"
#include "Basic.IDecoderFactory.h"
#include "Basic.ISingleByteEncodingIndex.h"

namespace Basic
{
	class SingleByteDecoder : public IDecoder
	{
	private:
		Ref<IStream<Codepoint> > destination; // $$$
		Ref<ISingleByteEncodingIndex> index; // $$$

		void Emit(Codepoint codepoint);
		void EmitDecoderError(byte b);
		void EmitDecoderError(const char* error);

	public:
		typedef Basic::Ref<SingleByteDecoder, IDecoder> Ref;

		void Initialize(ISingleByteEncodingIndex* index);
		void Initialize(ISingleByteEncodingIndex* index, IStream<Codepoint>* destination);

		void IDecoder::set_destination(IStream<Codepoint>* destination);
		void IDecoder::Write(const byte* elements, uint32 count);
		void IDecoder::WriteEOF();
	};

	class SingleByteDecoderFactory : public IDecoderFactory
	{
	private:
		Ref<ISingleByteEncodingIndex> index; // $$$

	public:
		typedef Ref<SingleByteDecoderFactory, IDecoderFactory> Ref;

		void Initialize(ISingleByteEncodingIndex* index);

		virtual void IDecoderFactory::CreateDecoder(Basic::Ref<IDecoder>* decoder);
	};
}