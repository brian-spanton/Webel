// Copyright © 2013 Brian Spanton

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

		UnicodeString::Ref json_false; // REF
		UnicodeString::Ref json_null; // REF
		UnicodeString::Ref json_true; // REF
		UnicodeString::Ref equals_method; // REF
		UnicodeString::Ref starts_with_method; // REF
		UnicodeString::Ref children_count_equals_method; // REF
		UnicodeString::Ref first_text_method; // REF
		UnicodeString::Ref deep_text_method; // REF
		UnicodeString::Ref text_equals_method; // REF

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

		UnicodeString::Ref ws; // REF

		BoolToken::Ref false_token; // REF
		NullToken::Ref null_token; // REF
		BoolToken::Ref true_token; // REF
	};

	extern Globals* globals;
}