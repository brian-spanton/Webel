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

    class Link
    {
    public:
        std::shared_ptr<ElementNode> element;
        std::shared_ptr<Uri> url;
        UnicodeStringRef text;
        bool is_iframe = false;

        virtual void Initialize(std::shared_ptr<ElementNode> element, std::shared_ptr<Uri> document_url);
    };

    typedef std::vector<std::shared_ptr<Link> > LinkList;
}
