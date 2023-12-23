#pragma once

#include "Html.TagToken.h"

namespace Html
{
	using namespace Basic;

	class StartTagToken : public TagToken
	{
	public:
		typedef Basic::Ref<StartTagToken> Ref;

		StartTagToken();

		virtual void GetDebugString(char* debug_string, int count);
	};
}