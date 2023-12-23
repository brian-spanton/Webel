#pragma once

#include "Basic.IDecoder.h"
#include "Basic.Ref.h"
#include "Html.Types.h"

namespace Basic
{
	class Utf32LittleEndianDecoder : public IDecoder
	{
	private:
		Ref<IStream<Codepoint> > destination; // $$$
		uint8 received;
		Codepoint codepoint;

		void Emit(Codepoint codepoint);

	public:
		typedef Basic::Ref<Utf32LittleEndianDecoder> Ref;

		void IDecoder::set_destination(IStream<Codepoint>* destination);
		void IDecoder::Write(const byte* elements, uint32 count);
		void IDecoder::WriteEOF();
	};

	class Utf32LittleEndianDecoderFactory : public IDecoderFactory
	{
	public:
		typedef Ref<Utf32LittleEndianDecoderFactory> Ref;

		virtual void IDecoderFactory::CreateDecoder(Basic::Ref<IDecoder>* decoder);
	};
}