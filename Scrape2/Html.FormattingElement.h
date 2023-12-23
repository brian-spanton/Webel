// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.StartTagToken.h"
#include "Html.ElementNode.h"

namespace Html
{
    using namespace Basic;

    class FormattingElement
    {
    public:
        std::shared_ptr<ElementNode> element;
        std::shared_ptr<TagToken> token;

        void Initialize(std::shared_ptr<ElementNode> element, std::shared_ptr<TagToken> token);
        bool equals(ElementNode* element, TagToken* token);
        bool IsMarker();
    };

    typedef std::vector<std::shared_ptr<FormattingElement> > FormattingElementList;
}