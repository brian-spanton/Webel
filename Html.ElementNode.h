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
		StringMap::Ref attributes; // REF

	public:
		typedef Basic::Ref<ElementNode> Ref;

		ElementName::Ref element_name; // REF
		ElementNode* form_owner;

		ElementNode();

		void Initialize(ElementName* element_name, StringMap* attributes);
		bool IsFormatting();
		bool IsMathMLTextIntegrationPoint();
		bool IsHTMLIntegrationPoint();
		bool IsFormAssociated();
		bool IsSubmittable();
		bool IsReassociateable();
		bool has_attribute_value(UnicodeString* attribute_name, UnicodeString* value);
		void write_html_to_human(IStream<Codepoint>* stream);
		virtual void write_to_human(IStream<Codepoint>* stream, bool verbose);

		bool has_element_name(ElementName* element_name);
		bool is_in_namespace(UnicodeString* namespace_name);
		bool has_attribute(UnicodeString* attribute_name);
		bool is_disabled();
		bool attribute_equals(UnicodeString* attribute_name, UnicodeString* value);
		void get_attribute(UnicodeString* attribute_name, UnicodeString::Ref* value);
		bool checkedness();
		bool attribute_missing_or_empty(UnicodeString* attribute_name);
		void set_attribute(UnicodeString* attribute_name, UnicodeString* attribute_value);
		uint32 get_attribute_count();
		void remove_attribute(UnicodeString* attribute_name);
	};

	typedef std::vector<Basic::Ref<ElementNode> > ElementList; // REF
}