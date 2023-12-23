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

		UnicodeString::Ref name; // $$$
		UnicodeString::Ref public_identifier; // $$$
		UnicodeString::Ref system_identifier; // $$$

		bool force_quirks;

		DocTypeToken();

		bool HasNameOf(ElementName* element);
	};
}