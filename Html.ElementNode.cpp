// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.ElementNode.h"
#include "Html.Globals.h"
#include "Basic.Globals.h"
#include "Basic.TextWriter.h"

namespace Html
{
	using namespace Basic;

	ElementNode::ElementNode() :
		Node(NodeType::ELEMENT_NODE)
	{
	}

	void ElementNode::Initialize(ElementName* element_name, StringMap* attributes)
	{
		this->element_name = element_name;
		this->attributes = attributes;
	}

	bool ElementNode::has_attribute(UnicodeString* attribute_name)
	{
		StringMap::iterator it = this->attributes->find(attribute_name);
		if (it == this->attributes->end())
			return false;

		return true;
	}

	bool ElementNode::has_attribute_value(UnicodeString* attribute_name, UnicodeString* value)
	{
		StringMap::iterator it = this->attributes->find(attribute_name);
		if (it == this->attributes->end())
			return false;

		if (it->second != value)
			return false;

		return true;
	}

	bool ElementNode::is_disabled()
	{
		//A form control is disabled if its disabled attribute is set,
		if (has_attribute(Html::globals->disabled_attribute_name))
			return true;

		//or if it is a descendant of a fieldset element whose disabled attribute is set and is not a descendant of that
		//fieldset element's first legend element child, if any.
		// $ NYI

		return false;
	}

	uint32 ElementNode::get_attribute_count()
	{
		return this->attributes->size();
	}

	void ElementNode::get_attribute(UnicodeString* attribute_name, UnicodeString::Ref* value)
	{
		StringMap::iterator it = this->attributes->find(attribute_name);
		if (it == this->attributes->end())
		{
			(*value) = (UnicodeString*)0;
			return;
		}

		(*value) = it->second;
	}

	bool ElementNode::checkedness()
	{
		if (has_attribute(Html::globals->checked_attribute_name))
			return true;

		return false;
	}

	bool ElementNode::attribute_missing_or_empty(UnicodeString* attribute_name)
	{
		StringMap::iterator it = this->attributes->find(attribute_name);
		if (it == this->attributes->end())
			return true;

		if (it->second->size() == 0)
			return true;

		return false;
	}

	bool ElementNode::has_element_name(ElementName* element_name)
	{
		return this->element_name->equals(element_name);
	}

	bool ElementNode::IsFormatting()
	{
		if (has_element_name(Html::globals->HTML_a))
			return true;
		
		if (has_element_name(Html::globals->HTML_b))
			return true;
		
		if (has_element_name(Html::globals->HTML_big))
			return true;
		
		if (has_element_name(Html::globals->HTML_code))
			return true;
		
		if (has_element_name(Html::globals->HTML_em))
			return true;
		
		if (has_element_name(Html::globals->HTML_font))
			return true;
		
		if (has_element_name(Html::globals->HTML_i))
			return true;
		
		if (has_element_name(Html::globals->HTML_nobr))
			return true;
		
		if (has_element_name(Html::globals->HTML_s))
			return true;
		
		if (has_element_name(Html::globals->HTML_small))
			return true;
		
		if (has_element_name(Html::globals->HTML_strike))
			return true;
		
		if (has_element_name(Html::globals->HTML_strong))
			return true;
		
		if (has_element_name(Html::globals->HTML_tt))
			return true;
		
		if (has_element_name(Html::globals->HTML_u))
			return true;

		return false;
	}

	bool ElementNode::is_in_namespace(UnicodeString* namespace_name)
	{
		return this->element_name->is_in_namespace(namespace_name);
	}

	bool ElementNode::IsFormAssociated()
	{
		if (has_element_name(Html::globals->button_element_name))
			return true;

		if (has_element_name(Html::globals->HTML_fieldset))
			return true;

		if (has_element_name(Html::globals->input_element_name)) 
			return true;

		if (has_element_name(Html::globals->HTML_keygen)) 
			return true;

		if (has_element_name(Html::globals->HTML_label)) 
			return true;

		if (has_element_name(Html::globals->object_element_name)) 
			return true;

		if (has_element_name(Html::globals->HTML_output)) 
			return true;

		if (has_element_name(Html::globals->select_element_name)) 
			return true;

		if (has_element_name(Html::globals->HTML_textarea))
			return true;

		return false;
 	}

	bool ElementNode::IsSubmittable()
	{
		if (has_element_name(Html::globals->button_element_name))
			return true;

		if (has_element_name(Html::globals->input_element_name)) 
			return true;

		if (has_element_name(Html::globals->HTML_keygen)) 
			return true;

		if (has_element_name(Html::globals->object_element_name)) 
			return true;

		if (has_element_name(Html::globals->select_element_name)) 
			return true;

		if (has_element_name(Html::globals->HTML_textarea))
			return true;

		return false;
 	}

	bool ElementNode::IsReassociateable()
	{
		if (has_element_name(Html::globals->button_element_name))
			return true;

		if (has_element_name(Html::globals->HTML_fieldset))
			return true;

		if (has_element_name(Html::globals->input_element_name)) 
			return true;

		if (has_element_name(Html::globals->HTML_keygen)) 
			return true;

		if (has_element_name(Html::globals->HTML_label)) 
			return true;

		if (has_element_name(Html::globals->object_element_name)) 
			return true;

		if (has_element_name(Html::globals->HTML_output)) 
			return true;

		if (has_element_name(Html::globals->select_element_name)) 
			return true;

		if (has_element_name(Html::globals->HTML_textarea))
			return true;

		return false;
 	}

	bool ElementNode::IsMathMLTextIntegrationPoint()
	{
		if (has_element_name(Html::globals->MathML_mi))
			return true;

		if (has_element_name(Html::globals->MathML_mo))
			return true;

		if (has_element_name(Html::globals->MathML_mn))
			return true;

		if (has_element_name(Html::globals->MathML_ms))
			return true;

		if (has_element_name(Html::globals->MathML_mtext))
			return true;

		return false;
	}

	bool ElementNode::IsHTMLIntegrationPoint()
	{
		if (has_element_name(Html::globals->MathML_annotation_xml))
		{
			if (has_attribute_value(Html::globals->encoding_attribute_name, Html::globals->text_html_media_type))
				return true;
		}

		if (has_element_name(Html::globals->MathML_annotation_xml))
		{
			if (has_attribute_value(Html::globals->encoding_attribute_name, Html::globals->application_xhtml_xml_media_type))
				return true;
		}

		if (has_element_name(Html::globals->SVG_foreignObject))
			return true;

		if (has_element_name(Html::globals->SVG_desc))
			return true;

		if (has_element_name(Html::globals->SVG_title))
			return true;

		return false;
	}

	void ElementNode::write_html_to_human(IStream<Codepoint>* stream)
	{
		Basic::TextWriter writer(stream);

		writer.Write("<");
		this->element_name->name->write_to(stream);
		if (this->attributes->size() == 0)
		{
			writer.Write("/>");
		}
		else
		{
			for (Html::StringMap::iterator it_attr = this->attributes->begin(); it_attr != this->attributes->end(); it_attr++)
			{
				writer.Write(" ");
				it_attr->first->write_to(stream);
				writer.Write("=\"");
				it_attr->second->write_to(stream);
				writer.Write("\"");
			}

			writer.Write(">");
		}
	}

	void ElementNode::write_to_human(IStream<Codepoint>* stream, bool verbose)
	{
		this->element_name->name->write_to(stream);

		if (verbose)
		{
			Basic::TextWriter writer(stream);

			for (Html::StringMap::iterator it_attr = this->attributes->begin(); it_attr != this->attributes->end(); it_attr++)
			{
				writer.Write(" ");
				it_attr->first->write_to(stream);
				writer.Write("=\"");
				it_attr->second->write_to(stream);
				writer.Write("\"");
			}
		}
	}

	bool ElementNode::attribute_equals(UnicodeString* attribute_name, UnicodeString* value)
	{
		UnicodeString::Ref attribute_value;
		get_attribute(attribute_name, &attribute_value);
		return attribute_value.equals<true>(value);
	}

	void ElementNode::set_attribute(UnicodeString* attribute_name, UnicodeString* attribute_value)
	{
		this->attributes->set_string(attribute_name, attribute_value);
	}

	void ElementNode::remove_attribute(UnicodeString* attribute_name)
	{
		this->attributes->erase(attribute_name);
	}
}
