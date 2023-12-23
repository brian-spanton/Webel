#pragma once

#include "Html.Node.h"
#include "Html.Types.h"
#include "Html.DocumentTypeNode.h"
#include "Basic.Uri.h"

namespace Html
{
	using namespace Basic;

	class Document : public Node
	{
	public:
		enum Mode
		{
			quirks_mode,
			limited_quirks_mode,
			no_quirks_mode,
		};

		typedef Basic::Ref<Document> Ref;

	public:
		DocumentTypeNode::Ref doctype; // $$$
		Mode mode;
		Uri::Ref url; // $$$

		Document();
		void Initialize(Uri::Ref url);
		virtual void Append(Node* node);
	};
}