// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Ref.h"
#include "Html.Types.h"

namespace Html
{
    using namespace Basic;

    class Token : public IRefCounted
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

        typedef Basic::Ref<Token> Ref;

        Type type;

        Token(Type type);

        virtual void GetDebugString(char* debug_string, int count);
    };

    typedef Token* TokenPointer;
}