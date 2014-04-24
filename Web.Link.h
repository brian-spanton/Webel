// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.IProcess.h"
#include "Html.ElementNode.h"
#include "Basic.Uri.h"

namespace Web
{
    using namespace Basic;
    using namespace Html;

    class Link : public IRefCounted
    {
    public:
        typedef Basic::Ref<Link> Ref;

        Basic::Ref<ElementNode> element; // REF
        Uri::Ref url; // REF
        UnicodeString::Ref text; // REF

        void Initialize(ElementNode* element, Uri* document_url);
    };

    typedef std::vector<Link::Ref> LinkList; // REF
}
