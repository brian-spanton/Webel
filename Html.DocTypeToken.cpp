// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.DocTypeToken.h"

namespace Html
{
	using namespace Basic;

	DocTypeToken::DocTypeToken() :
		Token(Token::Type::DOCTYPE_token),
		force_quirks(false)
	{
	}

	bool DocTypeToken::HasNameOf(ElementName* element)
	{
		return this->name.equals<true>(element->name);
	}
}