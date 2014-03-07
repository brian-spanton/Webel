// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Node.h"
#include "Html.Types.h"

namespace Html
{
	using namespace Basic;

	class DocumentTypeNode : public Node
	{
	public:
		typedef Basic::Ref<DocumentTypeNode> Ref;

		UnicodeString::Ref name; // REF
		UnicodeString::Ref publicId; // REF
		UnicodeString::Ref systemId; // REF

		DocumentTypeNode();
	};
}