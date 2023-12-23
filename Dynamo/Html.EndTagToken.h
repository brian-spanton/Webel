#pragma once

#include "Html.TagToken.h"

namespace Html
{
	using namespace Basic;

	class EndTagToken : public TagToken
	{
	public:
		typedef Basic::Ref<EndTagToken> Ref;

		EndTagToken();

		virtual void GetDebugString(char* debug_string, int count);
	};
}