#pragma once

#include "Basic.IEncoder.h"
#include "Basic.Ref.h"
#include "Html.Types.h"

namespace Basic
{
	class Utf8Encoder : public IEncoder
	{
	private:
		Ref<IStream<byte> > destination; // $$$
		byte error_replacement_byte;

		void Emit(byte b);
		void EncoderError(Codepoint codepoint);

	public:
		typedef Basic::Ref<Utf8Encoder> Ref;

		Utf8Encoder();

		void IEncoder::set_destination(IStream<byte>* destination);
		void IEncoder::set_error_replacement_byte(byte b);
		void IEncoder::Write(const Codepoint* elements, uint32 count);
		void IEncoder::WriteEOF();
	};

	class Utf8EncoderFactory : public IEncoderFactory
	{
	public:
		typedef Ref<Utf8EncoderFactory> Ref;

		virtual void IEncoderFactory::CreateEncoder(Basic::Ref<IEncoder>* encoder);
	};
}