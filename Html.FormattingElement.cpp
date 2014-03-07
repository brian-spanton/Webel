// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.FormattingElement.h"

namespace Html
{
	using namespace Basic;

	void FormattingElement::Initialize(ElementNode* element, TagToken* token)
	{
		this->element = element;
		this->token = token;
	}

	bool FormattingElement::equals(ElementNode* element, TagToken* token)
	{
		if (!this->element->has_element_name(element->element_name))
			return false;

		if (this->element->get_attribute_count() != element->get_attribute_count())
			return false;

		// $ NYI:
		// the attributes must be compared as they were when the elements were created by the parser;
		// two elements have the same attributes if all their parsed attributes can be paired such that the two attributes
		// in each pair have identical names, namespaces, and values (the order of the attributes does not matter).

		return true;
	}

	bool FormattingElement::IsMarker()
	{
		if (this->element.item() == 0)
			return true;

		return false;
	}
}