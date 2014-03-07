// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Node.h"
#include "Html.Types.h"

namespace Html
{
	using namespace Basic;

	class CommentNode : public Node
	{
	public:
		typedef Basic::Ref<CommentNode> Ref;

		UnicodeString::Ref data; // REF

		CommentNode();
	};
}