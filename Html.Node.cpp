// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.Globals.h"
#include "Html.Globals.h"
#include "Html.Node.h"
#include "Html.TextNode.h"
#include "Html.ElementNode.h"
#include "Basic.TextWriter.h"
#include "Basic.TextSanitizer.h"

namespace Html
{
    using namespace Basic;

    Node::Node(NodeType type) :
        type(type)
    {
    }

    void Node::Insert(Codepoint codepoint)
    {
        if (this->children.size() == 0 || this->children.back()->type != TEXT_NODE)
        {
            std::shared_ptr<TextNode> text_node = std::make_shared<TextNode>();
            Append(text_node);
        }

        TextNode* text_node = static_cast<TextNode*>(this->children.back().get());
        text_node->data->push_back(codepoint);
    }

    void Node::Append(std::shared_ptr<Node> node)
    {
        node->parent = this->shared_from_this();
        node->html_document = this->html_document;
        this->children.push_back(node);
    }

    void Node::Remove(std::shared_ptr<Node> node)
    {
        for (NodeList::iterator it = this->children.begin(); it != this->children.end(); it++)
        {
            if (*it == node)
            {
                this->children.erase(it);
                return;
            }
        }
    }

    bool Node::has_ancestor(std::shared_ptr<Node> node)
    {
        std::shared_ptr<Node> parent(this->parent);

        if (parent == node)
            return true;

        if (!parent)
            return false;

        return parent->has_ancestor(node);
    }

    bool Node::has_ancestor(ElementName* element_name)
    {
        std::shared_ptr<Node> parent(this->parent);

        if (!parent)
            return false;

        if (parent->type == NodeType::ELEMENT_NODE)
        {
            if (((ElementNode*)parent.get())->has_element_name(element_name))
                return true;
        }

        return parent->has_ancestor(element_name);
    }

    void Node::remove_from_parent()
    {
        std::shared_ptr<Node> parent = this->parent.lock();

        if (parent)
            parent->Remove(shared_from_this());
    }

    void Node::take_all_child_nodes_of(Node* node)
    {
        for (uint32 index = 0; index < node->children.size(); index++)
        {
            Append(node->children.at(index));
        }
    }

    void Node::write_to_human(IStream<Codepoint>* stream, bool verbose)
    {
        TextWriter writer(stream);
        writer.WriteFormat<0x10>("%d", this->type);
    }

    void Node::write_pointer_to_human(IStream<Codepoint>* stream)
    {
        TextWriter writer(stream);
        writer.WriteFormat<0x10>("%08X", (unsigned long)this);
    }

    bool Node::find_element_with_child_count(Node* after, ElementName* name, uint32 child_count, std::shared_ptr<ElementNode>* result)
    {
        if (after == 0)
        {
            if (this->type == NodeType::ELEMENT_NODE)
            {
                std::shared_ptr<ElementNode> element = std::static_pointer_cast<ElementNode>(shared_from_this());

                if (element->has_element_name(name) && element->children.size() == child_count)
                {
                    (*result) = element;
                    return true;
                }
            }
        }

        for (NodeList::iterator it = this->children.begin(); it != this->children.end(); it++)
        {
            if (after != 0)
            {
                if ((*it).get() == after)
                {
                    after = 0;
                }
                else if (after->has_ancestor(*it))
                {
                    bool found = (*it)->find_element_with_child_count(after, name, child_count, result);
                    if (found)
                        return true;

                    after = 0;
                }
            }
            else
            {
                bool found = (*it)->find_element_with_child_count(after, name, child_count, result);
                if (found)
                    return true;
            }
        }

        return false;
    }

    bool Node::find_element_with_attribute_prefix(Node* after, ElementName* name, UnicodeStringRef attribute, UnicodeStringRef value, std::shared_ptr<ElementNode>* result)
    {
        if (after == 0)
        {
            if (this->type == NodeType::ELEMENT_NODE)
            {
                std::shared_ptr<ElementNode> element = std::static_pointer_cast<ElementNode>(shared_from_this());

                if (element->has_element_name(name) &&
                    element->has_attribute(attribute))
                {
                    UnicodeStringRef attribute_value;
                    element->get_attribute(attribute, &attribute_value);

                    if (attribute_value->starts_with<true>(value.get()))
                    {
                        (*result) = element;
                        return true;
                    }
                }
            }
        }

        for (NodeList::iterator it = this->children.begin(); it != this->children.end(); it++)
        {
            if (after != 0)
            {
                if ((*it).get() == after)
                {
                    after = 0;
                }
                else if (after->has_ancestor(*it))
                {
                    bool found = (*it)->find_element_with_attribute_prefix(after, name, attribute, value, result);
                    if (found)
                        return true;

                    after = 0;
                }
            }
            else
            {
                bool found = (*it)->find_element_with_attribute_prefix(after, name, attribute, value, result);
                if (found)
                    return true;
            }
        }

        return false;
    }

    bool Node::find_element_with_attribute_value(Node* after, ElementName* name, UnicodeStringRef attribute, UnicodeStringRef value, std::shared_ptr<ElementNode>* result)
    {
        if (after == 0)
        {
            if (this->type == NodeType::ELEMENT_NODE)
            {
                std::shared_ptr<ElementNode> element = std::static_pointer_cast<ElementNode>(shared_from_this());

                if (element->has_element_name(name) &&
                    element->has_attribute_value(attribute, value))
                {
                    (*result) = element;
                    return true;
                }
            }
        }

        for (NodeList::iterator it = this->children.begin(); it != this->children.end(); it++)
        {
            if (after != 0)
            {
                if ((*it).get() == after)
                {
                    after = 0;
                }
                else if (after->has_ancestor(*it))
                {
                    bool found = (*it)->find_element_with_attribute_value(after, name, attribute, value, result);
                    if (found)
                        return true;

                    after = 0;
                }
            }
            else
            {
                bool found = (*it)->find_element_with_attribute_value(after, name, attribute, value, result);
                if (found)
                    return true;
            }
        }

        return false;
    }

    bool Node::find_element_with_text_value(Node* after, ElementName* name, UnicodeStringRef value, std::shared_ptr<ElementNode>* result)
    {
        if (after == 0)
        {
            if (this->type == NodeType::ELEMENT_NODE)
            {
                std::shared_ptr<ElementNode> element = std::static_pointer_cast<ElementNode>(shared_from_this());

                if (element->has_element_name(name))
                {
                    UnicodeStringRef text = std::make_shared<UnicodeString>();
                    text->reserve(0x100);

                    std::shared_ptr<TextSanitizer> stream = std::make_shared<TextSanitizer>();
                    stream->Initialize(text.get());

                    element->extract_text(stream.get());

                    if (equals<UnicodeString, true>(text.get(), value.get()))
                    {
                        (*result) = element;
                        return true;
                    }
                }
            }
        }

        for (NodeList::iterator it = this->children.begin(); it != this->children.end(); it++)
        {
            if (after != 0)
            {
                if ((*it).get() == after)
                {
                    after = 0;
                }
                else if (after->has_ancestor(*it))
                {
                    bool found = (*it)->find_element_with_text_value(after, name, value, result);
                    if (found)
                        return true;

                    after = 0;
                }
            }
            else
            {
                bool found = (*it)->find_element_with_text_value(after, name, value, result);
                if (found)
                    return true;
            }
        }

        return false;
    }

    bool Node::find_element(ElementName* name, UnicodeStringRef attribute_name, UnicodeStringRef attribute_value, std::shared_ptr<ElementNode>* result)
    {
        if (this->type == NodeType::ELEMENT_NODE)
        {
            std::shared_ptr<ElementNode> element = std::static_pointer_cast<ElementNode>(shared_from_this());

            if (element->has_element_name(name))
            {
                if (attribute_name && attribute_value)
                {
                    if (element->has_attribute_value(attribute_name, attribute_value))
                    {
                        (*result) = element;
                        return true;
                    }
                }
                else if (attribute_name)
                {
                    if (element->has_attribute(attribute_name))
                    {
                        (*result) = element;
                        return true;
                    }
                }
                else
                {
                    (*result) = element;
                    return true;
                }
            }
        }

        for (NodeList::iterator it = this->children.begin(); it != this->children.end(); it++)
        {
            bool found = (*it)->find_element(name, attribute_name, attribute_value, result);
            if (found)
                return true;
        }

        return false;
    }

    void Node::extract_text(IStream<Codepoint>* stream)
    {
        if (this->type == Html::NodeType::TEXT_NODE)
        {
            Html::TextNode* element = (Html::TextNode*)this;
            element->write_to_human(stream, false);
            return;
        }

        for (Node::NodeList::iterator it = this->children.begin(); it != this->children.end(); it++)
        {
            (*it)->extract_text(stream);
        }
    }

    bool Node::first_text(IStream<Codepoint>* stream)
    {
        if (this->type == Html::NodeType::TEXT_NODE)
        {
            Html::TextNode* element = (Html::TextNode*)this;
            element->write_to_human(stream, false);
            return true;
        }

        for (Node::NodeList::iterator it = this->children.begin(); it != this->children.end(); it++)
        {
            bool success = (*it)->first_text(stream);
            if (success)
                return true;
        }

        return false;
    }
}