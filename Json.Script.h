// Copyright © 2013 Brian Spanton

#pragma once

// $$ need to decouple json from html, yet enable json-html script

#include "Html.Node.h"
#include "Json.Types.h"

namespace Json
{
    class Script
    {
    private:
        void deep_text(Html::Node::Ref domain, UnicodeString::Ref* result);

    public:
        Basic::UnicodeString::Ref element_name; // REF
        Basic::UnicodeString::Ref attribute_name; // REF
        Basic::UnicodeString::Ref method_name; // REF
        Value::Ref parameter_value; // REF

        void Initialize();
        bool Execute(Html::Node::Ref domain, Html::Node::Ref after, Html::Node::Ref* result);
        bool Execute(Html::Node::Ref domain, UnicodeString::Ref* result);
    };
}