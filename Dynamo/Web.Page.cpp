#include "stdafx.h"
#include "Web.Page.h"
#include "Web.TextSanitizer.h"
#include "Html.Globals.h"
#include "Html.TextNode.h"
#include "Basic.CountStream.h"

namespace Web
{
	void Page::Initialize(Document* document, Http::Client::Ref client)
	{
		this->document = document;
		this->http_cookies = client->http_cookies;

		render(document);
	}

	void Page::render(Node* node)
	{
		NodeList children = node->children;

		for (Html::Node::NodeList::iterator it = children.begin(); it != children.end(); it++)
		{
			render(*it);
		}

		bool prune = true;

		if (node->type == Html::NodeType::ELEMENT_NODE)
		{
			Html::ElementNode* element = (Html::ElementNode*)node;

			if (element->has_element_name(Html::globals->HTML_form))
			{
				Form::Ref item = New<Form>();
				item->Initialize(element);
				this->forms.push_back(item);
				prune = false;
			}
			else if (element->has_element_name(Html::globals->HTML_a))
			{
				Link::Ref item = New<Link>();
				item->Initialize(element, this->document->url);
				this->links.push_back(item);
				prune = false;
			}
			else if (element->has_element_name(Html::globals->HTML_img))
			{
				prune = false;
			}
			else if (element->form_owner != 0)
			{
				prune = false;
			}
		}
		else if (node->type == Html::NodeType::TEXT_NODE)
		{
			Html::TextNode* element = (Html::TextNode*)node;

			Inline<CountStream<Codepoint> > count_stream;

			Inline<TextSanitizer> stream;
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

	bool Page::find_link(UnicodeString::Ref pattern, Web::Link::Ref* result)
	{
		for (uint16 i = 0; i != this->links.size(); i++)
		{
			if (this->links[i]->text->equals<false>(pattern))
			{
				(*result) = this->links[i];
				return true;
			}
		}

		return false;
	}

	bool Page::find_form(UnicodeString::Ref pattern, Web::Form::Ref* result)
	{
		for (uint16 i = 0; i != this->forms.size(); i++)
		{
			UnicodeString::Ref id;
			this->forms[i]->form_element->get_attribute(Html::globals->id_attribute_name, &id);

			if (id.equals<false>(pattern))
			{
				(*result) = this->forms[i];
				return true;
			}
		}

		return false;
	}
}