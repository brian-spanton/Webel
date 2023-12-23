#include "stdafx.h"
#include "Html.TagToken.h"

namespace Html
{
	using namespace Basic;

	TagToken::TagToken(Type type) :
		Token(type)
	{
		this->self_closing = false;
		this->acknowledged = false;
		this->name = New<UnicodeString>();
		this->attributes = New<StringMap>();
	}

	bool TagToken::HasNameOf(ElementName* element)
	{
		return this->name.equals<true>(element->name);
	}
}