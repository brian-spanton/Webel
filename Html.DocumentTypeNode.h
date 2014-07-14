// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Node.h"
#include "Html.Types.h"

namespace Html
{
    using namespace Basic;

    class DocumentTypeNode : public Node
    {
    public:
        UnicodeStringRef name;
        UnicodeStringRef publicId;
        UnicodeStringRef systemId;

        DocumentTypeNode();
    };
}