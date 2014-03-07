// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Json.Parser.h"
#include "Json.Globals.h"

namespace Json
{
	using namespace Basic;

	void Parser::Initialize(Html::Node::Ref domain, UnicodeString::Ref charset)
	{
		this->text = New<Text>();
		this->text->Initialize(domain);

		FrameStream<Token::Ref>::Ref text_stream = New<FrameStream<Token::Ref> >();
		text_stream->Initialize(this->text);

		this->tokenizer = New<Tokenizer>();
		this->tokenizer->Initialize(text_stream);

		FrameStream<Codepoint>::Ref tokenizer_stream = New<FrameStream<Codepoint> >();
		tokenizer_stream->Initialize(this->tokenizer);

		this->decoder = New<ByteStreamDecoder>();
		this->decoder->Initialize(charset, tokenizer_stream);

		__super::Initialize(this->decoder);
	}

	bool Parser::ParseError(const char* error)
	{
		std::string parse_error = "Parse error: ";
		parse_error += error;
		return Basic::globals->HandleError(parse_error.c_str(), 0);
	}
}