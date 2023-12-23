// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Token.h"

namespace Html
{
    using namespace Basic;

    class CommentToken : public Token
    {
    public:
        UnicodeStringRef data;

        CommentToken();
    };
}