#include "stdafx.h"
#include "Html.EndOfFileToken.h"

namespace Html
{
	using namespace Basic;

	EndOfFileToken::EndOfFileToken() :
		Token(Token::Type::end_of_file_token)
	{
	}
}