#pragma once

#include "Html.Document.h"
#include "Web.Link.h"
#include "Web.Form.h"
#include "Http.Types.h"

namespace Web
{
	using namespace Basic;
	using namespace Html;

	class Page : public IRefCounted
	{
	private:
		void render(Node* node);

	public:
		typedef Basic::Ref<Page> Ref;
		typedef std::vector<Node::Ref> NodeList; // $$$

		Basic::Ref<Document> document; // $$$
		LinkList links;
		FormList forms;
		NodeList leaf_nodes; // $$$
		Http::CookieList http_cookies;

		void Initialize(Document* document, Http::Client::Ref client);
		bool find_link(UnicodeString::Ref pattern, Web::Link::Ref* result);
		bool find_form(UnicodeString::Ref pattern, Web::Form::Ref* result);
	};
}
