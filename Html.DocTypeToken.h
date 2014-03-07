// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Token.h"
#include "Html.ElementName.h"

namespace Html
{
	using namespace Basic;

	class DocTypeToken : public Token
	{
	public:
		typedef Basic::Ref<DocTypeToken> Ref;

		UnicodeString::Ref name; // REF
		UnicodeString::Ref public_identifier; // REF
		UnicodeString::Ref system_identifier; // REF

		bool force_quirks;

		DocTypeToken();

		bool HasNameOf(ElementName* element);
	};
}