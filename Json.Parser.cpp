// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Json.Parser.h"
#include "Json.Globals.h"

namespace Json
{
    using namespace Basic;

    void Parser::Initialize(std::shared_ptr<Html::Node> domain, UnicodeStringRef charset)
    {
        this->text = std::make_shared<Text>(domain);
        this->tokenizer = std::make_shared<Tokenizer>(this->text);
        this->decoder = std::make_shared<ByteStreamDecoder>(charset, this->tokenizer.get());

        __super::Initialize(this->decoder.get());
    }

    bool Parser::ParseError(const char* error)
    {
        std::string parse_error = "Parse error: ";
        parse_error += error;
        return Basic::globals->HandleError(parse_error.c_str(), 0);
    }
}