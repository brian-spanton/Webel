// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Web.Page.h"
#include "Basic.TextSanitizer.h"
#include "Html.Globals.h"
#include "Html.TextNode.h"
#include "Basic.CountStream.h"

namespace Web
{
    void Page::Initialize(std::shared_ptr<Document> document, std::shared_ptr<Web::Client> client)
    {
        this->document = document;
        this->http_cookies = client->http_cookies;

        render(document);
    }

    void Page::render(std::shared_ptr<Node> node)
    {
        NodeList children = node->children;

        for (Html::Node::NodeList::iterator it = children.begin(); it != children.end(); it++)
        {
            render(*it);
        }

        bool prune = true;

        if (node->type == Html::NodeType::ELEMENT_NODE)
        {
            std::shared_ptr<Html::ElementNode> element = std::static_pointer_cast<Html::ElementNode>(node);

            if (element->has_element_name(Html::globals->HTML_form.get()))
            {
                std::shared_ptr<Form> item = std::make_shared<Form>();
                item->Initialize(element);
                this->forms.push_back(item);
                prune = false;
            }
            else if (element->has_element_name(Html::globals->HTML_a.get()))
            {
                std::shared_ptr<Link> item = std::make_shared<Link>();
                item->Initialize(element, this->document->url);
                this->links.push_back(item);
                prune = false;
            }
            else if (element->has_element_name(Html::globals->HTML_img.get()))
            {
                prune = false;
            }
            else if (element->form_owner.lock().get() != 0)
            {
                prune = false;
            }
        }
        else if (node->type == Html::NodeType::TEXT_NODE)
        {
            Html::TextNode* element = (Html::TextNode*)node.get();

            CountStream<Codepoint> count_stream;
            TextSanitizer stream;
            stream.Initialize(&count_stream);

            element->write_to_human(&stream, false);

            if (count_stream.count > 0)
            {
                prune = false;
            }
        }

        if (node->children.size() == 0)
        {
            if (prune)
            {
                node->remove_from_parent();
            }
            else
            {
                this->leaf_nodes.push_back(node);
            }
        }
    }

    bool Page::find_link(UnicodeStringRef pattern, std::shared_ptr<Web::Link>* result)
    {
        for (uint16 i = 0; i != this->links.size(); i++)
        {
            if (equals<UnicodeString, false>(this->links[i]->text.get(), pattern.get()))
            {
                (*result) = this->links[i];
                return true;
            }
        }

        return false;
    }

    bool Page::find_form(UnicodeStringRef pattern, std::shared_ptr<Web::Form>* result)
    {
        for (uint16 i = 0; i != this->forms.size(); i++)
        {
            UnicodeStringRef id;
            this->forms[i]->form_element->get_attribute(Html::globals->id_attribute_name, &id);

            if (equals<UnicodeString, false>(id.get(), pattern.get()))
            {
                (*result) = this->forms[i];
                return true;
            }
        }

        return false;
    }
}