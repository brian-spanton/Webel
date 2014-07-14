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

    void ElementNode::Initialize(std::shared_ptr<ElementName> element_name, std::shared_ptr<StringMap> attributes)
    {
        this->element_name = element_name;
        this->attributes = attributes;
    }

    bool ElementNode::has_attribute(UnicodeStringRef attribute_name)
    {
        StringMap::iterator it = this->attributes->find(attribute_name);
        if (it == this->attributes->end())
            return false;

        return true;
    }

    bool ElementNode::has_attribute_value(UnicodeStringRef attribute_name, UnicodeStringRef value)
    {
        StringMap::iterator it = this->attributes->find(attribute_name);
        if (it == this->attributes->end())
            return false;

        if (!equals<UnicodeString, true>(it->second.get(), value.get()))
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

    void ElementNode::get_attribute(UnicodeStringRef attribute_name, UnicodeStringRef* value)
    {
        StringMap::iterator it = this->attributes->find(attribute_name);
        if (it == this->attributes->end())
        {
            (*value) = std::shared_ptr<UnicodeString>();
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

    bool ElementNode::attribute_missing_or_empty(UnicodeStringRef attribute_name)
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
        if (has_element_name(Html::globals->HTML_a.get()))
            return true;
        
        if (has_element_name(Html::globals->HTML_b.get()))
            return true;
        
        if (has_element_name(Html::globals->HTML_big.get()))
            return true;
        
        if (has_element_name(Html::globals->HTML_code.get()))
            return true;
        
        if (has_element_name(Html::globals->HTML_em.get()))
            return true;
        
        if (has_element_name(Html::globals->HTML_font.get()))
            return true;
        
        if (has_element_name(Html::globals->HTML_i.get()))
            return true;
        
        if (has_element_name(Html::globals->HTML_nobr.get()))
            return true;
        
        if (has_element_name(Html::globals->HTML_s.get()))
            return true;
        
        if (has_element_name(Html::globals->HTML_small.get()))
            return true;
        
        if (has_element_name(Html::globals->HTML_strike.get()))
            return true;
        
        if (has_element_name(Html::globals->HTML_strong.get()))
            return true;
        
        if (has_element_name(Html::globals->HTML_tt.get()))
            return true;
        
        if (has_element_name(Html::globals->HTML_u.get()))
            return true;

        return false;
    }

    bool ElementNode::is_in_namespace(UnicodeStringRef namespace_name)
    {
        return this->element_name->is_in_namespace(namespace_name.get());
    }

    bool ElementNode::IsFormAssociated()
    {
        if (has_element_name(Html::globals->button_element_name.get()))
            return true;

        if (has_element_name(Html::globals->HTML_fieldset.get()))
            return true;

        if (has_element_name(Html::globals->input_element_name.get()))
            return true;

        if (has_element_name(Html::globals->HTML_keygen.get()))
            return true;

        if (has_element_name(Html::globals->HTML_label.get()))
            return true;

        if (has_element_name(Html::globals->object_element_name.get()))
            return true;

        if (has_element_name(Html::globals->HTML_output.get()))
            return true;

        if (has_element_name(Html::globals->select_element_name.get()))
            return true;

        if (has_element_name(Html::globals->HTML_textarea.get()))
            return true;

        return false;
     }

    bool ElementNode::IsSubmittable()
    {
        if (has_element_name(Html::globals->button_element_name.get()))
            return true;

        if (has_element_name(Html::globals->input_element_name.get()))
            return true;

        if (has_element_name(Html::globals->HTML_keygen.get()))
            return true;

        if (has_element_name(Html::globals->object_element_name.get()))
            return true;

        if (has_element_name(Html::globals->select_element_name.get()))
            return true;

        if (has_element_name(Html::globals->HTML_textarea.get()))
            return true;

        return false;
     }

    bool ElementNode::IsReassociateable()
    {
        if (has_element_name(Html::globals->button_element_name.get()))
            return true;

        if (has_element_name(Html::globals->HTML_fieldset.get()))
            return true;

        if (has_element_name(Html::globals->input_element_name.get()))
            return true;

        if (has_element_name(Html::globals->HTML_keygen.get()))
            return true;

        if (has_element_name(Html::globals->HTML_label.get()))
            return true;

        if (has_element_name(Html::globals->object_element_name.get()))
            return true;

        if (has_element_name(Html::globals->HTML_output.get()))
            return true;

        if (has_element_name(Html::globals->select_element_name.get()))
            return true;

        if (has_element_name(Html::globals->HTML_textarea.get()))
            return true;

        return false;
     }

    bool ElementNode::IsMathMLTextIntegrationPoint()
    {
        if (has_element_name(Html::globals->MathML_mi.get()))
            return true;

        if (has_element_name(Html::globals->MathML_mo.get()))
            return true;

        if (has_element_name(Html::globals->MathML_mn.get()))
            return true;

        if (has_element_name(Html::globals->MathML_ms.get()))
            return true;

        if (has_element_name(Html::globals->MathML_mtext.get()))
            return true;

        return false;
    }

    bool ElementNode::IsHTMLIntegrationPoint()
    {
        if (has_element_name(Html::globals->MathML_annotation_xml.get()))
        {
            if (has_attribute_value(Html::globals->encoding_attribute_name, Html::globals->text_html_media_type))
                return true;
        }

        if (has_element_name(Html::globals->MathML_annotation_xml.get()))
        {
            if (has_attribute_value(Html::globals->encoding_attribute_name, Html::globals->application_xhtml_xml_media_type))
                return true;
        }

        if (has_element_name(Html::globals->SVG_foreignObject.get()))
            return true;

        if (has_element_name(Html::globals->SVG_desc.get()))
            return true;

        if (has_element_name(Html::globals->SVG_title.get()))
            return true;

        return false;
    }

    void ElementNode::write_html_to_human(IStream<Codepoint>* stream)
    {
        Basic::TextWriter writer(stream);

        writer.write_literal("<");
        this->element_name->name->write_to_stream(stream);
        if (this->attributes->size() == 0)
        {
            writer.write_literal("/>");
        }
        else
        {
            for (Html::StringMap::iterator it_attr = this->attributes->begin(); it_attr != this->attributes->end(); it_attr++)
            {
                writer.write_literal(" ");
                it_attr->first->write_to_stream(stream);
                writer.write_literal("=\"");
                it_attr->second->write_to_stream(stream);
                writer.write_literal("\"");
            }

            writer.write_literal(">");
        }
    }

    void ElementNode::write_to_human(IStream<Codepoint>* stream, bool verbose)
    {
        this->element_name->name->write_to_stream(stream);

        if (verbose)
        {
            Basic::TextWriter writer(stream);

            for (Html::StringMap::iterator it_attr = this->attributes->begin(); it_attr != this->attributes->end(); it_attr++)
            {
                writer.write_literal(" ");
                it_attr->first->write_to_stream(stream);
                writer.write_literal("=\"");
                it_attr->second->write_to_stream(stream);
                writer.write_literal("\"");
            }
        }
    }

    bool ElementNode::attribute_equals(UnicodeStringRef attribute_name, UnicodeStringRef value)
    {
        UnicodeStringRef attribute_value;
        get_attribute(attribute_name, &attribute_value);
        return equals<UnicodeString, true>(attribute_value.get(), value.get());
    }

    void ElementNode::set_attribute(UnicodeStringRef attribute_name, UnicodeStringRef attribute_value)
    {
        this->attributes->set_string(attribute_name, attribute_value);
    }

    void ElementNode::remove_attribute(UnicodeStringRef attribute_name)
    {
        this->attributes->erase(attribute_name);
    }
}
