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
        this->element_name = 0;
        this->attribute_name = 0;
        this->method_name = 0;
        this->parameter_value = 0;
    }

    bool Script::Execute(std::shared_ptr<Html::Node> domain, std::shared_ptr<Html::Node> after, std::shared_ptr<Html::Node>* result)
    {
        if (this->element_name.get() != 0)
        {
            std::shared_ptr<Html::ElementName> element_name = std::make_shared<Html::ElementName>();
            element_name->Initialize(Html::globals->Namespace_HTML, this->element_name);

            if (this->attribute_name.get() == 0 &&
                equals<UnicodeString, true>(this->method_name.get(), Json::globals->children_count_equals_method.get()) &&
                this->parameter_value.get() != 0 &&
                this->parameter_value->type == Json::Value::Type::number_value)
            {
                Json::Number* number_parameter = (Json::Number*)this->parameter_value.get();

                std::shared_ptr<Html::ElementNode> element_node;
                bool success = domain->find_element_with_child_count(after.get(), element_name.get(), (uint32)number_parameter->value, &element_node);
                if (success)
                {
                    (*result) = element_node;
                    return true;
                }
            }
            else if (this->attribute_name.get() != 0 &&
                equals<UnicodeString, true>(this->method_name.get(), Json::globals->starts_with_method.get()) &&
                this->parameter_value.get() != 0 &&
                this->parameter_value->type == Json::Value::Type::string_value)
            {
                Json::String* string_parameter = (Json::String*)this->parameter_value.get();

                std::shared_ptr<Html::ElementNode> element_node;
                bool success = domain->find_element_with_attribute_prefix(after.get(), element_name.get(), this->attribute_name, string_parameter->value, &element_node);
                if (success)
                {
                    (*result) = element_node;
                    return true;
                }
            }
            else if (this->attribute_name.get() != 0 &&
                equals<UnicodeString, true>(this->method_name.get(), Json::globals->equals_method.get()) &&
                this->parameter_value.get() != 0 &&
                this->parameter_value->type == Json::Value::Type::string_value)
            {
                Json::String* string_parameter = (Json::String*)this->parameter_value.get();

                std::shared_ptr<Html::ElementNode> element_node;
                bool success = domain->find_element_with_attribute_value(after.get(), element_name.get(), this->attribute_name, string_parameter->value, &element_node);
                if (success)
                {
                    (*result) = element_node;
                    return true;
                }
            }
            else if (this->attribute_name.get() == 0 &&
                equals<UnicodeString, true>(this->method_name.get(), Json::globals->text_equals_method.get()) &&
                this->parameter_value.get() != 0 &&
                this->parameter_value->type == Json::Value::Type::string_value)
            {
                Json::String* string_parameter = (Json::String*)this->parameter_value.get();

                std::shared_ptr<Html::ElementNode> element_node;
                bool success = domain->find_element_with_text_value(after.get(), element_name.get(), string_parameter->value, &element_node);
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

    bool Script::Execute(std::shared_ptr<Html::Node> domain, UnicodeStringRef* result)
    {
        if (this->element_name.get() != 0)
        {
            std::shared_ptr<Html::ElementName> element_name = std::make_shared<Html::ElementName>();
            element_name->Initialize(Html::globals->Namespace_HTML, this->element_name);

            if (this->attribute_name.get() != 0 &&
                this->method_name.get() == 0)
            {
                std::shared_ptr<Html::ElementNode> element_node;
                bool success = domain->find_element(element_name.get(), this->attribute_name, 0, &element_node);
                if (success)
                {
                    element_node->get_attribute(this->attribute_name, result);
                    return true;
                }
            }
        }
        else
        {
            if (this->attribute_name.get() == 0 &&
                this->method_name.get() != 0 &&
                this->parameter_value.get() == 0)
            {
                if (equals<UnicodeString, true>(this->method_name.get(), Json::globals->deep_text_method.get()))
                {
                    deep_text(domain, result);
                    return true;
                }
                else if (equals<UnicodeString, true>(this->method_name.get(), Json::globals->first_text_method.get()))
                {
                    UnicodeStringRef text = std::make_shared<UnicodeString>();
                    text->reserve(0x100);

                    Basic::TextSanitizer stream;
                    stream.Initialize(text.get());

                    domain->first_text(&stream);

                    (*result) = text;
                    return true;
                }
            }
        }

        std::shared_ptr<Html::Node> element_node;
        bool success = Execute(domain, 0, &element_node);
        if (!success)
            return false;

        deep_text(element_node, result);

        return true;
    }

    void Script::deep_text(std::shared_ptr<Html::Node> domain, UnicodeStringRef* result)
    {
        UnicodeStringRef text = std::make_shared<UnicodeString>();
        text->reserve(0x100);

        Basic::TextSanitizer stream;
        stream.Initialize(text.get());

        domain->extract_text(&stream);

        (*result) = text;
    }
}