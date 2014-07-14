// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Node.h"
#include "Html.Types.h"

namespace Html
{
    using namespace Basic;

    class TextNode : public Node
    {
    public:
        UnicodeStringRef data;

        TextNode();

        virtual void write_to_human(IStream<Codepoint>* stream, bool verbose);
    };
}