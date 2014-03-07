// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.StartTagToken.h"

namespace Html
{
	using namespace Basic;

	StartTagToken::StartTagToken() :
		TagToken(Token::Type::start_tag_token)
	{
	}

	void StartTagToken::GetDebugString(char* debug_string, int count)
	{
		std::string name;
		name.insert(name.end(), this->name->begin(), this->name->begin() + this->name->size());
		sprintf_s(debug_string, count, "<%s>", name.c_str());
	}
}