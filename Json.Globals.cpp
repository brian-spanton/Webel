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
        this->begin_script = '<';
        this->end_script = '>';
        this->begin_parameter = '(';
        this->end_parameter = ')';
        this->token_separator = '.';
        this->begin_array = '[';
        this->begin_object = '{';
        this->end_array = ']';
        this->end_object = '}';
        this->name_separator = ':';
        this->value_separator = ',';

        this->json_false.Initialize("false");
        this->json_null.Initialize("null");
        this->json_true.Initialize("true");
        this->equals_method.Initialize("equals");
        this->starts_with_method.Initialize("starts_with");
        this->children_count_equals_method.Initialize("children_count_equals");
        this->first_text_method.Initialize("first_text");
        this->deep_text_method.Initialize("deep_text");
        this->text_equals_method.Initialize("text_equals");

        const char ws_chars[] = { 0x20, 0x09, 0x0A, 0x0D, };
        this->ws.Initialize(ws_chars);

        this->false_token = New<BoolToken>();
        this->false_token->value = false;

        this->true_token = New<BoolToken>();
        this->true_token->value = true;

        this->null_token = New<NullToken>();

        Tokenizer::InitializeStatics();
    }
}