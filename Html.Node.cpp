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
		type(type),
		parent(0)
	{
	}

	void Node::Insert(Codepoint c)
	{
		if (this->children.size() == 0 || this->children.back()->type != TEXT_NODE)
		{
			TextNode::Ref text_node = New<TextNode>();
			Append(text_node.item());
		}

		TextNode* text_node = static_cast<TextNode*>(this->children.back().item());
		text_node->data->push_back(c);
	}

	void Node::Append(Node* node)
	{
		node->parent = this;
		node->html_document = this->html_document;
		this->children.push_back(node);
	}

	void Node::Remove(Node* node)
	{
		for (NodeList::iterator it = this->children.begin(); it != this->children.end(); it++)
		{
			if (it->item() == node)
			{
				this->children.erase(it);
				return;
			}
		}
	}

	bool Node::has_ancestor(Node* node)
	{
		if (this->parent == node)
			return true;

		if (this->parent == 0)
			return false;

		return this->parent->has_ancestor(node);
	}

	bool Node::has_ancestor(ElementName* element_name)
	{
		if (this->parent == 0)
			return false;

		if (this->parent->type == NodeType::ELEMENT_NODE)
		{
			if (((ElementNode*)this->parent)->has_element_name(element_name))
				return true;
		}

		return this->parent->has_ancestor(element_name);
	}

	void Node::remove_from_parent()
	{
		if (this->parent != 0)
			this->parent->Remove(this);
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

	bool Node::find_element_with_child_count(Node* after, ElementName* name, uint32 child_count, Basic::Ref<ElementNode>* result)
	{
		if (after == 0)
		{
			if (this->type == NodeType::ELEMENT_NODE)
			{
				ElementNode* element = (ElementNode*)this;

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
				if ((*it).item() == after)
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

	bool Node::find_element_with_attribute_prefix(Node* after, ElementName* name, UnicodeString* attribute, UnicodeString* value, Basic::Ref<ElementNode>* result)
	{
		if (after == 0)
		{
			if (this->type == NodeType::ELEMENT_NODE)
			{
				ElementNode* element = (ElementNode*)this;

				if (element->has_element_name(name) &&
					element->has_attribute(attribute))
				{
					UnicodeString::Ref attribute_value;
					element->get_attribute(attribute, &attribute_value);

					if (attribute_value->starts_with<true>(value))
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
				if ((*it).item() == after)
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

	bool Node::find_element_with_attribute_value(Node* after, ElementName* name, UnicodeString* attribute, UnicodeString* value, Basic::Ref<ElementNode>* result)
	{
		if (after == 0)
		{
			if (this->type == NodeType::ELEMENT_NODE)
			{
				ElementNode* element = (ElementNode*)this;

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
				if ((*it).item() == after)
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

	bool Node::find_element_with_text_value(Node* after, ElementName* name, UnicodeString* value, Basic::Ref<ElementNode>* result)
	{
		if (after == 0)
		{
			if (this->type == NodeType::ELEMENT_NODE)
			{
				ElementNode* element = (ElementNode*)this;

				if (element->has_element_name(name))
				{
					UnicodeString::Ref text = New<UnicodeString>();
					text->reserve(0x100);

					Inline<TextSanitizer> stream;
					stream.Initialize(text);

					element->extract_text(&stream);

					if (text.equals<true>(value))
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
				if ((*it).item() == after)
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

	bool Node::find_element(ElementName* name, UnicodeString* attribute_name, UnicodeString* attribute_value, Basic::Ref<ElementNode>* result)
	{
		if (this->type == NodeType::ELEMENT_NODE)
		{
			ElementNode* element = (ElementNode*)this;

			if (element->has_element_name(name))
			{
				if (attribute_name != 0 && attribute_value != 0)
				{
					if (element->has_attribute_value(attribute_name, attribute_value))
					{
						(*result) = element;
						return true;
					}
				}
				else if (attribute_name != 0)
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

	void Node::extract_text(IStream<Codepoint>* destination)
	{
		if (this->type == Html::NodeType::TEXT_NODE)
		{
			Html::TextNode* element = (Html::TextNode*)this;
			element->write_to_human(destination, false);
			return;
		}

		for (Node::NodeList::iterator it = this->children.begin(); it != this->children.end(); it++)
		{
			(*it)->extract_text(destination);
		}
	}

	bool Node::first_text(IStream<Codepoint>* destination)
	{
		if (this->type == Html::NodeType::TEXT_NODE)
		{
			Html::TextNode* element = (Html::TextNode*)this;
			element->write_to_human(destination, false);
			return true;
		}

		for (Node::NodeList::iterator it = this->children.begin(); it != this->children.end(); it++)
		{
			bool success = (*it)->first_text(destination);
			if (success)
				return true;
		}

		return false;
	}
}