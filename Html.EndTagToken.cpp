// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.EndTagToken.h"

namespace Html
{
    using namespace Basic;

    EndTagToken::EndTagToken() :
        TagToken(Token::Type::end_tag_token)
    {
    }

    void EndTagToken::GetDebugString(char* debug_string, int count)
    {
        ByteString ascii_string;
        ascii_encode(this->name.get(), &ascii_string);
        sprintf_s(debug_string, count, "</%s>", (char*)ascii_string.c_str());
    }
}