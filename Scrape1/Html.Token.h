// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Types.h"

namespace Html
{
    using namespace Basic;

    class Token
    {
    public:
        enum Type
        {
            DOCTYPE_token,
            start_tag_token,
            end_tag_token,
            comment_token,
            character_token,
            end_of_file_token,
        };

        Type type;

        Token(Type type);

        virtual void GetDebugString(char* debug_string, int count) const;
    };

    typedef std::shared_ptr<Token> TokenRef;
}