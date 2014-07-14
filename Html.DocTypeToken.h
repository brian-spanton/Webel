// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Token.h"
#include "Html.ElementName.h"

namespace Html
{
    using namespace Basic;

    class DocTypeToken : public Token
    {
    public:
        UnicodeStringRef name;
        UnicodeStringRef public_identifier;
        UnicodeStringRef system_identifier;

        bool force_quirks;

        DocTypeToken();

        bool has_name_of(ElementName* element);
    };
}