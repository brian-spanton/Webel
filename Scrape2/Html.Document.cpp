// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.Document.h"

namespace Html
{
    using namespace Basic;

    Document::Document(std::shared_ptr<Uri> url) :
        Node(NodeType::DOCUMENT_NODE),
        url(url)
    {
    }

    void Document::Append(std::shared_ptr<Node> node)
    {
        __super::Append(node);
        node->html_document = std::static_pointer_cast<Document>(this->shared_from_this());
    }
}