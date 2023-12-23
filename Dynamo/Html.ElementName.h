#pragma once

#include "Html.Types.h"

namespace Html
{
	using namespace Basic;

	class ElementName : public IRefCounted
	{
	public:
		typedef Basic::Ref<ElementName> Ref;

		UnicodeString::Ref name_space; // $$$
		UnicodeString::Ref name; // $$$

		ElementName();

		void Initialize(ElementName* element_name);
		void Initialize(UnicodeString* name_space, UnicodeString* name);

		bool equals(ElementName* element_name);

		bool is_in_namespace(UnicodeString* name_space);
	};

	class ElementNameList : public std::vector<ElementName::Ref>, public IRefCounted // $$$
	{
	public:
		typedef Basic::Ref<ElementNameList> Ref;
	};
}