#pragma once

#include "Html.Types.h"
#include "Html.ByteStreamDecoder.h"
#include "Html.InputStreamPreprocessor.h"
#include "Html.Tokenizer.h"
#include "Html.TreeConstruction.h"
#include "Basic.FrameStream.h"
#include "Http.Client.h"

namespace Html
{
	using namespace Basic;

	class Parser : public FrameStream<byte>
	{
	private:
		friend class ByteStreamDecoder;
		friend class InputStreamPreprocessor;
		friend class Tokenizer;
		friend class TreeConstruction;
		friend class CharacterReferenceFrame;

		ByteStreamDecoder::Ref decoder; // $$$
		InputStreamPreprocessor::Ref preprocessor; // $$$
		Tokenizer::Ref tokenizer; // $$$

		bool ParseError(const char* error);

	public:
		typedef Basic::Ref<Parser, IStream<byte> > Ref;

		TreeConstruction::Ref tree; // $$$

		void Initialize(Http::Client::Ref client);
		void Initialize(Uri::Ref url, UnicodeString::Ref charset);
	};
}