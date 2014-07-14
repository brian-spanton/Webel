// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Token.h"
#include "Html.Types.h"
#include "Html.ElementName.h"

namespace Html
{
    using namespace Basic;

    class TagToken : public Token
    {
    public:
        UnicodeStringRef name;
        bool self_closing;
        bool acknowledged;
        StringMap attributes;

        TagToken(Type type);

        bool has_name_of(ElementName* element);
    };
}