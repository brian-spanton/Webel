// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.IProcess.h"
#include "Html.ElementNode.h"
#include "Basic.Uri.h"
#include "Web.Link.h"

namespace Web
{
    using namespace Basic;
    using namespace Html;

    class IFrame : public Link
    {
    public:
        virtual void Initialize(std::shared_ptr<ElementNode> element, std::shared_ptr<Uri> document_url);
    };
}
