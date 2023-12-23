// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Node.h"
#include "Html.Types.h"
#include "Html.ElementName.h"

namespace Html
{
    using namespace Basic;

    class ElementNode : public Node
    {
    private:
        std::shared_ptr<StringMap> attributes;

    public:
        std::shared_ptr<ElementName> element_name;
        std::weak_ptr<ElementNode> form_owner;

        ElementNode();

        void Initialize(std::shared_ptr<ElementName> element_name, std::shared_ptr<StringMap> attributes);
        bool IsFormatting();
        bool IsMathMLTextIntegrationPoint();
        bool IsHTMLIntegrationPoint();
        bool IsFormAssociated();
        bool IsSubmittable();
        bool IsReassociateable();
        bool has_attribute_value(UnicodeStringRef attribute_name, UnicodeStringRef value);
        void write_html_to_human(IStream<Codepoint>* stream);
        virtual void write_to_human(IStream<Codepoint>* stream, bool verbose);

        bool has_element_name(ElementName* element_name);
        bool is_in_namespace(UnicodeStringRef namespace_name);
        bool has_attribute(UnicodeStringRef attribute_name);
        bool is_disabled();
        bool attribute_equals(UnicodeStringRef attribute_name, UnicodeStringRef value);
        void get_attribute(UnicodeStringRef attribute_name, UnicodeStringRef* value);
        bool checkedness();
        bool attribute_missing_or_empty(UnicodeStringRef attribute_name);
        void set_attribute(UnicodeStringRef attribute_name, UnicodeStringRef attribute_value);
        uint32 get_attribute_count();
        void remove_attribute(UnicodeStringRef attribute_name);
    };

    typedef std::vector<std::shared_ptr<ElementNode> > ElementList;
}