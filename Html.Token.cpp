// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.Token.h"
#include "Html.ElementName.h"
#include "Basic.Globals.h"

namespace Html
{
    using namespace Basic;

    Token::Token(Type type) :
        type(type)
    {
    }

    void Token::GetDebugString(char* debug_string, int count) const
    {
        switch (this->type)
        {
#define CASE(e) \
        case e: \
            strcpy_s(debug_string, count, #e); \
            break

            CASE(DOCTYPE_token);
            CASE(start_tag_token);
            CASE(end_tag_token);
            CASE(comment_token);
            CASE(character_token);
            CASE(end_of_file_token);

#undef CASE

        default:
            sprintf_s(debug_string, count, "%d", this->type);
            Basic::LogError("Html", "Token::GetDebugString unexpected type");
            break;
        }
    }
}