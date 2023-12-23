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
        void deep_text(std::shared_ptr<Html::Node> domain, UnicodeStringRef* result);

    public:
        Basic::UnicodeStringRef element_name;
        Basic::UnicodeStringRef attribute_name;
        Basic::UnicodeStringRef method_name;
        std::shared_ptr<Value> parameter_value;

        void Initialize();
        bool Execute(std::shared_ptr<Html::Node> domain, std::shared_ptr<Html::Node> after, std::shared_ptr<Html::Node>* result);
        bool Execute(std::shared_ptr<Html::Node> domain, UnicodeStringRef* result);
    };
}