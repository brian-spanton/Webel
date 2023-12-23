#pragma once

#include "Html.Token.h"

namespace Html
{
	using namespace Basic;

	class CommentToken : public Token
	{
	public:
		typedef Basic::Ref<CommentToken> Ref;

		UnicodeString::Ref data; // $$$

		CommentToken();
	};
}