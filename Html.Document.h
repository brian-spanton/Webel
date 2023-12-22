// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Node.h"
#include "Html.Types.h"
#include "Html.DocumentTypeNode.h"
#include "Basic.Uri.h"

namespace Html
{
    using namespace Basic;

    class Document : public Node
    {
    public:
        enum Mode
        {
            quirks_mode,
            limited_quirks_mode,
            no_quirks_mode,
        };

    public:
        std::shared_ptr<DocumentTypeNode> doctype;
        Mode mode = Mode::quirks_mode;
        std::shared_ptr<Uri> url;

        Document(std::shared_ptr<Uri> url);

        virtual void Append(std::shared_ptr<Node> node);
    };
}