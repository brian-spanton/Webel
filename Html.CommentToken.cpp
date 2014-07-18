// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.CommentToken.h"

namespace Html
{
    using namespace Basic;

    CommentToken::CommentToken() :
        Token(Token::Type::comment_token),
        data(std::make_shared<UnicodeString>())
    {
    }
}