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

        UnicodeStringRef json_false;
        UnicodeStringRef json_null;
        UnicodeStringRef json_true;
        UnicodeStringRef equals_method;
        UnicodeStringRef starts_with_method;
        UnicodeStringRef children_count_equals_method;
        UnicodeStringRef first_text_method;
        UnicodeStringRef deep_text_method;
        UnicodeStringRef text_equals_method;

        static const Codepoint begin_script = '<';
        static const Codepoint end_script = '>';
        static const Codepoint begin_parameter = '(';
        static const Codepoint end_parameter = ')';
        static const Codepoint token_separator = '.';
        static const Codepoint begin_array = '[';
        static const Codepoint begin_object = '{';
        static const Codepoint end_array = ']';
        static const Codepoint end_object = '}';
        static const Codepoint name_separator = ':';
        static const Codepoint value_separator = ',';

        UnicodeStringRef ws;

        std::shared_ptr<BoolToken> false_token;
        std::shared_ptr<NullToken> null_token;
        std::shared_ptr<BoolToken> true_token;
    };

    extern Globals* globals;
}