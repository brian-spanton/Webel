#pragma once

#include "Basic.IDecoder.h"
#include "Basic.Ref.h"
#include "Html.Types.h"

namespace Basic
{
	class Utf8Decoder : public IDecoder
	{
	private:
		Ref<IStream<Codepoint> > destination; // $$$
		int needed;
		int seen;
		Codepoint codepoint;
		Codepoint lower_bound;
		Codepoint upper_bound;

		void Emit(Codepoint codepoint);
		void EmitDecoderError(byte b);
		void EmitDecoderError(const char* error);

	public:
		typedef Basic::Ref<Utf8Decoder> Ref;

		void IDecoder::set_destination(IStream<Codepoint>* destination);
		void IDecoder::Write(const byte* elements, uint32 count);
		void IDecoder::WriteEOF();
	};

	class Utf8DecoderFactory : public IDecoderFactory
	{
	public:
		typedef Ref<Utf8DecoderFactory> Ref;

		virtual void IDecoderFactory::CreateDecoder(Basic::Ref<IDecoder>* decoder);
	};
}