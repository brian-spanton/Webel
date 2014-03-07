// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Token.h"

namespace Html
{
	using namespace Basic;

	class CharacterToken : public Token
	{
	public:
		typedef Basic::Ref<CharacterToken> Ref;

		Codepoint data;

		CharacterToken();
	};
}