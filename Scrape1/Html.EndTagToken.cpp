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
        std::string name;
        name.insert(name.end(), this->name->begin(), this->name->begin() + this->name->size());
        sprintf_s(debug_string, count, "</%s>", name.c_str());
    }
}