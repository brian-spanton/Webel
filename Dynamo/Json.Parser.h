#pragma once

#include "Basic.FrameStream.h"
#include "Html.Types.h"
#include "Json.ByteStreamDecoder.h"
#include "Json.Tokenizer.h"
#include "Json.Text.h"
#include "Http.Client.h"

namespace Json
{
	using namespace Basic;

	class Parser : public FrameStream<byte>
	{
	private:
		friend class ByteStreamDecoder;
		friend class Tokenizer;
		friend class Text;

		ByteStreamDecoder::Ref decoder; // $$$
		Tokenizer::Ref tokenizer; // $$$

		bool ParseError(const char* error);

	public:
		typedef Basic::Ref<Parser, IStream<byte> > Ref;

		Text::Ref text; // $$$

		void Initialize(Html::Node::Ref domain, Http::Client::Ref client);
		void Initialize(Html::Node::Ref domain, UnicodeString::Ref charset);
	};
}