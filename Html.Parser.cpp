// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.Parser.h"
#include "Html.Globals.h"
#include "Html.Types.h"
#include "Html.StartTagToken.h"
#include "Html.EndTagToken.h"
#include "Html.CharacterToken.h"
#include "Html.EndOfFileToken.h"

namespace Html
{
	using namespace Basic;

	void Parser::Initialize(Uri::Ref url, UnicodeString::Ref charset)
	{
		this->tree = New<TreeConstruction>();
		this->tree->Initialize(this, url);

		this->tokenizer = New<Tokenizer>();
		this->tokenizer->Initialize(this, this->tree);

		FrameStream<Codepoint>::Ref tokenizer_stream = New<FrameStream<Codepoint> >();
		tokenizer_stream->Initialize(this->tokenizer);

		this->preprocessor = New<InputStreamPreprocessor>();
		this->preprocessor->Initialize(this, tokenizer_stream);

		this->decoder = New<ByteStreamDecoder>();
		this->decoder->Initialize(this, charset, this->preprocessor);

		__super::Initialize(this->decoder);
	}

	bool Parser::ParseError(const char* error)
	{
		std::string parse_error = "Parse error: ";
		parse_error += error;
		return Basic::globals->HandleError(parse_error.c_str(), 0);
	}
}