#pragma once

#include "Json.Types.h"

namespace Json
{
	using namespace Basic;

	class Globals
	{
	public:
		Globals();

		void Initialize();

		UnicodeString::Ref json_false; // $$$
		UnicodeString::Ref json_null; // $$$
		UnicodeString::Ref json_true; // $$$
		UnicodeString::Ref equals_method; // $$$
		UnicodeString::Ref starts_with_method; // $$$
		UnicodeString::Ref children_count_equals_method; // $$$
		UnicodeString::Ref first_text_method; // $$$
		UnicodeString::Ref deep_text_method; // $$$
		UnicodeString::Ref text_equals_method; // $$$

		Codepoint begin_script;
		Codepoint end_script;
		Codepoint begin_parameter;
		Codepoint end_parameter;
		Codepoint token_separator;
		Codepoint begin_array;
		Codepoint begin_object;
		Codepoint end_array;
		Codepoint end_object;
		Codepoint name_separator;
		Codepoint value_separator;

		UnicodeString::Ref ws; // $$$

		BoolToken::Ref false_token; // $$$
		NullToken::Ref null_token; // $$$
		BoolToken::Ref true_token; // $$$
	};

	extern Globals* globals;
}