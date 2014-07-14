// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.DocTypeToken.h"

namespace Html
{
    using namespace Basic;

    DocTypeToken::DocTypeToken() :
        Token(Token::Type::DOCTYPE_token),
        force_quirks(false)
    {
    }

    bool DocTypeToken::has_name_of(ElementName* element)
    {
        return equals<UnicodeString, true>(this->name.get(), element->name.get());
    }
}