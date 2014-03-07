// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.Document.h"

namespace Html
{
	using namespace Basic;

	Document::Document() :
		Node(NodeType::DOCUMENT_NODE)
	{
	}

	void Document::Initialize(Uri::Ref url)
	{
		this->url = url;
	}

	void Document::Append(Node* node)
	{
		__super::Append(node);
		node->html_document = this;
	}
}