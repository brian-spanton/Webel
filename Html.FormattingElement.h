// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.StartTagToken.h"
#include "Html.ElementNode.h"

namespace Html
{
    using namespace Basic;

    class FormattingElement : public IRefCounted
    {
    public:
        typedef Basic::Ref<FormattingElement> Ref;

        ElementNode::Ref element; // REF
        TagToken::Ref token; // REF

        void Initialize(ElementNode* element, TagToken* token);
        bool equals(ElementNode* element, TagToken* token);
        bool IsMarker();
    };

    typedef std::vector<FormattingElement::Ref> FormattingElementList; // REF
}