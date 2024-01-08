// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.Parser.h"
#include "Html.Globals.h"
#include "Html.Types.h"
#include "Html.StartTagToken.h"
#include "Html.EndTagToken.h"
#include "Html.CharacterToken.h"
#include "Html.EndOfFileToken.h"
#include "Basic.Globals.h"

namespace Html
{
    using namespace Basic;

    // initialization is in order of declaration in class def
    Parser::Parser(std::shared_ptr<Uri> url, UnicodeStringRef charset) :
        tree(std::make_shared<TreeConstruction>(this, url)),
        tokenizer(std::make_shared<Tokenizer>(this, this->tree)),
        preprocessor(std::make_shared<InputStreamPreprocessor>(this, this->tokenizer)),
        decoder(std::make_shared<ByteStreamDecoder>(this, charset, this->preprocessor))
    {
    }

    void Parser::write_element(byte b)
    {
        this->decoder->write_element(b);
    }

    void Parser::ParseError(const char* error)
    {
        std::string parse_error = "Parse error: ";
        parse_error += error;
        Basic::LogDebug("Html", parse_error.c_str());
    }
}