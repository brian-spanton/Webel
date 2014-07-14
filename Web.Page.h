// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Document.h"
#include "Web.Link.h"
#include "Web.Form.h"
#include "Http.Types.h"

namespace Web
{
    using namespace Basic;
    using namespace Html;

    class Page
    {
    private:
        void render(std::shared_ptr<Node> node);

    public:
        typedef std::vector<std::shared_ptr<Node> > NodeList;

        std::shared_ptr<Document> document;
        LinkList links;
        FormList forms;
        NodeList leaf_nodes;
        Http::CookieList http_cookies;

        void Initialize(std::shared_ptr<Document> document, std::shared_ptr<Web::Client> client);
        bool find_link(UnicodeStringRef pattern, std::shared_ptr<Web::Link>* result);
        bool find_form(UnicodeStringRef pattern, std::shared_ptr<Web::Form>* result);
    };
}
