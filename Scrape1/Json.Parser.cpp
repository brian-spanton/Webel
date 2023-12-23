// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Json.Parser.h"
#include "Json.Globals.h"

namespace Json
{
    using namespace Basic;

    Parser::Parser(std::shared_ptr<Html::Node> domain, UnicodeStringRef charset) :
        text(std::make_shared<Text>(domain)),
        tokenizer(std::make_shared<Tokenizer>(this->text)), // initialization is in order of declaration in class def
        decoder(std::make_shared<ByteStreamDecoder>(charset, this->tokenizer.get())) // initialization is in order of declaration in class def
    {
    }

    void Parser::write_element(byte b)
    {
        this->decoder->write_element(b);
    }
}