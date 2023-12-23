// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.CharacterToken.h"

namespace Html
{
    using namespace Basic;

    CharacterToken::CharacterToken() :
        Token(Token::Type::character_token)
    {
    }
}