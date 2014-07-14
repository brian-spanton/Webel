// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Json.Globals.h"
#include "Json.Tokenizer.h"

namespace Json
{
    // $ this is here because it must get constructed before Json::globals
    Tokenizer::LiteralMap Tokenizer::literal_map;

    Globals* globals = 0;

    Globals::Globals()
    {
    }

    void Globals::Initialize()
    {
        initialize_unicode(&this->json_false, "false");
        initialize_unicode(&this->json_null, "null");
        initialize_unicode(&this->json_true, "true");
        initialize_unicode(&this->equals_method, "equals");
        initialize_unicode(&this->starts_with_method, "starts_with");
        initialize_unicode(&this->children_count_equals_method, "children_count_equals");
        initialize_unicode(&this->first_text_method, "first_text");
        initialize_unicode(&this->deep_text_method, "deep_text");
        initialize_unicode(&this->text_equals_method, "text_equals");

        const char ws_chars[] = { 0x20, 0x09, 0x0A, 0x0D, };
        initialize_unicode(&this->ws, ws_chars);

        this->false_token = std::make_shared<BoolToken>();
        this->false_token->value = false;

        this->true_token = std::make_shared<BoolToken>();
        this->true_token->value = true;

        this->null_token = std::make_shared<NullToken>();

        Tokenizer::InitializeStatics();
    }
}