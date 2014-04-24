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
        typedef Basic::Ref<TagToken> Ref;

        UnicodeString::Ref name; // REF
        bool self_closing;
        bool acknowledged;
        StringMap::Ref attributes; // REF

        TagToken(Type type);

        bool HasNameOf(ElementName* element);
    };
}