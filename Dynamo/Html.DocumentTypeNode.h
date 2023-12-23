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

		UnicodeString::Ref name; // $$$
		UnicodeString::Ref publicId; // $$$
		UnicodeString::Ref systemId; // $$$

		DocumentTypeNode();
	};
}