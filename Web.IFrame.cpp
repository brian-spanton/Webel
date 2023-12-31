// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Web.IFrame.h"
#include "Html.Globals.h"
#include "Html.TextNode.h"
#include "Basic.TextSanitizer.h"

namespace Web
{
    void IFrame::Initialize(std::shared_ptr<ElementNode> element, std::shared_ptr<Uri> document_url)
    {
        this->element = element;

        UnicodeStringRef src_attribute_value;
        this->element->get_attribute(Html::globals->src_attribute_name, &src_attribute_value);

        if (is_null_or_empty(src_attribute_value.get()) == false)
        {
            std::shared_ptr<Uri> url = std::make_shared<Uri>();
            url->Initialize();

            bool success = url->Parse(src_attribute_value.get(), document_url.get());
            if (success)
            {
                this->url = url;
            }
        }

        this->text = std::make_shared<UnicodeString>();
        this->text->reserve(0x100);

        TextSanitizer stream;
        stream.Initialize(this->text.get());

        element->extract_text(&stream);

        if (this->text->size() == 0)
        {
            UnicodeStringRef id_attribute_value;
            this->element->get_attribute(Html::globals->id_attribute_name, &id_attribute_value);

            if (is_null_or_empty(id_attribute_value.get()) == false)
            {
                this->text->append(*id_attribute_value);
            }
        }
    }
}