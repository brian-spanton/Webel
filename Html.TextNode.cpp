// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.TextNode.h"
#include "Html.Globals.h"
#include "Html.ElementNode.h"

namespace Html
{
    using namespace Basic;

    TextNode::TextNode() :
        Node(NodeType::TEXT_NODE)
    {
        this->data = New<UnicodeString>();
    }

    void TextNode::write_to_human(IStream<Codepoint>* stream, bool verbose)
    {
        if (this->parent->type == NodeType::ELEMENT_NODE)
        {
            ElementNode* parent_element = (ElementNode*)this->parent;

            if (parent_element->has_element_name(Html::globals->HTML_script))
            {
                TextWriter writer(stream);
                writer.Write("...");
                return;
            }
        }

        if (verbose == false && this->data->size() > 100)
        {
            stream->Write(this->data->c_str(), 100);
        }
        else
        {
            this->data->write_to(stream);
        }
    }
}