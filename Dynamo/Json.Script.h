#pragma once

#include "Html.ElementNode.h"
#include "Json.Types.h"

namespace Json
{
	class Script
	{
	public:
		Basic::UnicodeString::Ref element_name; // $$$
		Basic::UnicodeString::Ref attribute_name; // $$$
		Basic::UnicodeString::Ref method_name; // $$$
		Value::Ref parameter_value; // $$$

		void Initialize();
		bool Execute(Html::Node* domain, Html::Node* after, Html::ElementNode::Ref* result);
		bool Execute(Html::Node* domain, UnicodeString::Ref* result);
		void deep_text(Html::Node* domain, UnicodeString::Ref* result);
	};
}