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

    void Parser::Initialize(std::shared_ptr<Uri> url, UnicodeStringRef charset)
    {
        this->tree = std::make_shared<TreeConstruction>(this, url);
        this->tokenizer = std::make_shared<Tokenizer>(this, this->tree);
        this->preprocessor = std::make_shared<InputStreamPreprocessor>(this, this->tokenizer);
        this->decoder = std::make_shared<ByteStreamDecoder>(this, charset, this->preprocessor);

        __super::Initialize(this->decoder.get());
    }

    bool Parser::ParseError(const char* error)
    {
        std::string parse_error = "Parse error: ";
        parse_error += error;
        return Basic::globals->HandleError(parse_error.c_str(), 0);
    }
}