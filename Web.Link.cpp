// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Web.Link.h"
#include "Html.Globals.h"
#include "Html.TextNode.h"
#include "Basic.TextSanitizer.h"

namespace Web
{
    void Link::Initialize(ElementNode* element, Uri* document_url)
    {
        this->element = element;

        UnicodeString::Ref href_attribute_value;
        this->element->get_attribute(Html::globals->href_attribute_name, &href_attribute_value);

        if (href_attribute_value.is_null_or_empty() == false)
        {
            Uri::Ref url = New<Uri>();
            url->Initialize();

            bool success = url->Parse(href_attribute_value, document_url);
            if (success)
            {
                this->url = url;
            }
        }

        this->text = New<UnicodeString>();
        this->text->reserve(0x100);

        Inline<TextSanitizer> stream;
        stream.Initialize(this->text);

        element->extract_text(&stream);
    }
}