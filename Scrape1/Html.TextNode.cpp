// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.TextNode.h"
#include "Html.Globals.h"
#include "Html.ElementNode.h"

namespace Html
{
    using namespace Basic;

    TextNode::TextNode() :
        Node(NodeType::TEXT_NODE),
        data(std::make_shared<UnicodeString>())
    {
    }

    void TextNode::write_to_human(IStream<Codepoint>* stream, bool verbose)
    {
        std::shared_ptr<Node> parent(this->parent);

        if (parent->type == NodeType::ELEMENT_NODE)
        {
            ElementNode* parent_element = (ElementNode*)parent.get();

            if (parent_element->has_element_name(Html::globals->HTML_script.get()))
            {
                TextWriter writer(stream);
                writer.write_literal("...");
                return;
            }
        }

        if (verbose == false && this->data->size() > 100)
        {
            stream->write_elements(this->data->address(), 100);
        }
        else
        {
            this->data->write_to_stream(stream);
        }
    }
}