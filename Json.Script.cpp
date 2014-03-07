// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Json.Script.h"
#include "Json.Globals.h"
#include "Html.Globals.h"
#include "Html.ElementNode.h"
#include "Basic.TextSanitizer.h"

namespace Json
{
	void Script::Initialize()
	{
		this->element_name = (UnicodeString*)0;
		this->attribute_name = (UnicodeString*)0;
		this->method_name = (UnicodeString*)0;
		this->parameter_value = (Value*)0;
	}

	bool Script::Execute(Html::Node::Ref domain, Html::Node::Ref after, Html::Node::Ref* result)
	{
		if (this->element_name.item() != 0)
		{
			Html::ElementName::Ref element_name = New<Html::ElementName>();
			element_name->Initialize(Html::globals->Namespace_HTML, this->element_name);

			if (this->attribute_name.item() == 0 &&
				this->method_name.equals<true>(Json::globals->children_count_equals_method) &&
				this->parameter_value.item() != 0 &&
				this->parameter_value->type == Json::Value::Type::number_value)
			{
				Json::Number* number_parameter = (Json::Number*)this->parameter_value.item();

				Html::ElementNode::Ref element_node;
				bool success = domain->find_element_with_child_count(after, element_name, (uint32)number_parameter->value, &element_node);
				if (success)
				{
					(*result) = element_node;
					return true;
				}
			}
			else if (this->attribute_name.item() != 0 &&
				this->method_name.equals<true>(Json::globals->starts_with_method) &&
				this->parameter_value.item() != 0 &&
				this->parameter_value->type == Json::Value::Type::string_value)
			{
				Json::String* string_parameter = (Json::String*)this->parameter_value.item();

				Html::ElementNode::Ref element_node;
				bool success = domain->find_element_with_attribute_prefix(after, element_name, this->attribute_name, string_parameter->value, &element_node);
				if (success)
				{
					(*result) = element_node;
					return true;
				}
			}
			else if (this->attribute_name.item() != 0 &&
				this->method_name.equals<true>(Json::globals->equals_method) &&
				this->parameter_value.item() != 0 &&
				this->parameter_value->type == Json::Value::Type::string_value)
			{
				Json::String* string_parameter = (Json::String*)this->parameter_value.item();

				Html::ElementNode::Ref element_node;
				bool success = domain->find_element_with_attribute_value(after, element_name, this->attribute_name, string_parameter->value, &element_node);
				if (success)
				{
					(*result) = element_node;
					return true;
				}
			}
			else if (this->attribute_name.item() == 0 &&
				this->method_name.equals<true>(Json::globals->text_equals_method) &&
				this->parameter_value.item() != 0 &&
				this->parameter_value->type == Json::Value::Type::string_value)
			{
				Json::String* string_parameter = (Json::String*)this->parameter_value.item();

				Html::ElementNode::Ref element_node;
				bool success = domain->find_element_with_text_value(after, element_name, string_parameter->value, &element_node);
				if (success)
				{
					(*result) = element_node;
					return true;
				}
			}
		}

		(*result) = 0;
		return false;
	}

	bool Script::Execute(Html::Node::Ref domain, UnicodeString::Ref* result)
	{
		if (this->element_name.item() != 0)
		{
			Html::ElementName::Ref element_name = New<Html::ElementName>();
			element_name->Initialize(Html::globals->Namespace_HTML, this->element_name);

			if (this->attribute_name.item() != 0 &&
				this->method_name.item() == 0)
			{
				Html::ElementNode::Ref element_node;
				bool success = domain->find_element(element_name, this->attribute_name, 0, &element_node);
				if (success)
				{
					element_node->get_attribute(this->attribute_name, result);
					return true;
				}
			}
		}
		else
		{
			if (this->attribute_name.item() == 0 &&
				this->method_name.item() != 0 &&
				this->parameter_value.item() == 0)
			{
				if (this->method_name.equals<true>(Json::globals->deep_text_method))
				{
					deep_text(domain, result);
					return true;
				}
				else if (this->method_name.equals<true>(Json::globals->first_text_method))
				{
					UnicodeString::Ref text = New<UnicodeString>();
					text->reserve(0x100);

					Inline<Basic::TextSanitizer> stream;
					stream.Initialize(text);

					domain->first_text(&stream);

					(*result) = text;
					return true;
				}
			}
		}

		Html::Node::Ref element_node;
		bool success = Execute(domain, 0, &element_node);
		if (!success)
			return false;

		deep_text(element_node, result);

		return true;
	}

	void Script::deep_text(Html::Node::Ref domain, UnicodeString::Ref* result)
	{
		UnicodeString::Ref text = New<UnicodeString>();
		text->reserve(0x100);

		Inline<Basic::TextSanitizer> stream;
		stream.Initialize(text);

		domain->extract_text(&stream);

		(*result) = text;
	}
}