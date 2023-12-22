// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Token.h"

namespace Html
{
    using namespace Basic;

    class CharacterToken : public Token
    {
    public:
        Codepoint data = 0;

        CharacterToken();
    };
}