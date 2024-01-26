// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Json.Tokenizer.h"
#include "Json.Globals.h"
#include "Json.Types.h"
#include "Json.Parser.h"
#include "Basic.Event.h"
#include "Basic.LogEntry.h"

namespace Json
{
    using namespace Basic;

    void Tokenizer::InitializeStatics()
    {
        literal_map.insert(LiteralMap::value_type(Json::globals->json_false, Json::globals->false_token));
        literal_map.insert(LiteralMap::value_type(Json::globals->json_null, Json::globals->null_token));
        literal_map.insert(LiteralMap::value_type(Json::globals->json_true, Json::globals->true_token));
    }

    Tokenizer::Tokenizer(std::shared_ptr<IStream<std::shared_ptr<Token> > > output) :
        output(output)
    {
    }

    void Tokenizer::Emit(std::shared_ptr<Token> token)
    {
        this->output->write_element(token);
    }

    void Tokenizer::handle_error(const char* error)
    {
        Basic::LogDebug("Json", "Tokenizer", "handle_error", error);
        switch_to_state(State::error_state);
    }

    void Tokenizer::write_element(Codepoint codepoint)
    {
        switch (get_state())
        {
        case State::start_state:
            {
                switch(codepoint)
                {
                case Json::globals->begin_script:
                    Emit(std::make_shared<BeginScriptToken>());
                    break;

                case Json::globals->end_script:
                    Emit(std::make_shared<EndScriptToken>());
                    break;

                case Json::globals->begin_parameter:
                    Emit(std::make_shared<BeginParameterToken>());
                    break;

                case Json::globals->end_parameter:
                    Emit(std::make_shared<EndParameterToken>());
                    break;

                case Json::globals->token_separator:
                    Emit(std::make_shared<TokenSeparatorToken>());
                    break;

                case Json::globals->begin_array:
                    Emit(std::make_shared<BeginArrayToken>());
                    break;

                case Json::globals->begin_object:
                    Emit(std::make_shared<BeginObjectToken>());
                    break;

                case Json::globals->end_array:
                    Emit(std::make_shared<EndArrayToken>());
                    break;

                case Json::globals->end_object:
                    Emit(std::make_shared<EndObjectToken>());
                    break;

                case Json::globals->name_separator:
                    Emit(std::make_shared<NameSeparatorToken>());
                    break;

                case Json::globals->value_separator:
                    Emit(std::make_shared<ValueSeparatorToken>());
                    break;

                case '\"':
                    {
                        this->string = std::make_shared<UnicodeString>();
                        this->string->reserve(0x40);
                        switch_to_state(State::string_state);
                    }
                    break;

                case '-':
                    {
                        this->sign = -1;
                        this->dec_number_stream.reset(&this->whole);
                        switch_to_state(State::number_state);
                    }
                    break;

                default:
                    {
                        if (codepoint >= '0' && codepoint <= '9')
                        {
                            this->sign = 1;
                            this->dec_number_stream.reset(&this->whole);

                            switch_to_state(State::number_state);
                            write_element(codepoint);
                            return;
                        }
                        else
                        {
                            for (this->literal_it = literal_map.begin(); this->literal_it != literal_map.end(); this->literal_it++)
                            {
                                if (codepoint == this->literal_it->first->at(0))
                                    break;
                            }

                            if (this->literal_it != literal_map.end())
                            {
                                this->matched = 1;
                                switch_to_state(State::literal_state);
                            }
                            else if (codepoint == Json::globals->token_separator || Json::globals->ws->find(codepoint) == UnicodeString::npos)
                            {
                                this->string = std::make_shared<UnicodeString>();
                                this->string->reserve(0x40);

                                switch_to_state(State::token_state);
                                write_element(codepoint);
                                return;
                            }
                        }
                    }
                    break;
                }
            }
            break;

        case State::literal_state:
            {
                if (codepoint == this->literal_it->first->at(this->matched))
                {
                    this->matched++;

                    if (this->matched == this->literal_it->first->size())
                    {
                        Emit(this->literal_it->second);
                        switch_to_state(State::start_state);
                    }
                }
                else
                {
                    this->string = std::make_shared<UnicodeString>();
                    this->string->reserve(0x40);
                    this->string->insert(this->string->end(), this->literal_it->first->begin(), this->literal_it->first->begin() + this->matched);

                    switch_to_state(State::token_state);
                    write_element(codepoint);
                    return;
                }
            }
            break;

        case State::token_state:
            {
                switch(codepoint)
                {
                case Json::globals->token_separator:
                    {
                        std::shared_ptr<TokenToken> token = std::make_shared<TokenToken>();
                        token->value = this->string;
                        Emit(token);

                        std::shared_ptr<TokenSeparatorToken> token2 = std::make_shared<TokenSeparatorToken>();
                        Emit(token2);

                        this->string = std::make_shared<UnicodeString>();
                        this->string->reserve(0x40);
                    }
                    break;

                case Json::globals->begin_parameter:
                    {
                        std::shared_ptr<TokenToken> token = std::make_shared<TokenToken>();
                        token->value = this->string;
                        Emit(token);

                        std::shared_ptr<BeginParameterToken> token2 = std::make_shared<BeginParameterToken>();
                        Emit(token2);

                        switch_to_state(State::start_state);
                    }
                    break;

                case Json::globals->end_script:
                    {
                        std::shared_ptr<TokenToken> token = std::make_shared<TokenToken>();
                        token->value = this->string;
                        Emit(token);

                        std::shared_ptr<EndScriptToken> token2 = std::make_shared<EndScriptToken>();
                        Emit(token2);

                        switch_to_state(State::start_state);
                    }
                    break;

                default:
                    {
                        if (Json::globals->ws->find(codepoint) != UnicodeString::npos)
                        {
                            std::shared_ptr<TokenToken> token = std::make_shared<TokenToken>();
                            token->value = this->string;
                            Emit(token);

                            switch_to_state(State::start_state);
                        }
                        else
                        {
                            this->string->push_back(codepoint);
                        }
                    }
                    break;
                }
            }
            break;

        case State::string_state:
            {
                if (codepoint == '\"')
                {
                    std::shared_ptr<StringToken> token = std::make_shared<StringToken>();
                    token->value = this->string;
                    Emit(token);

                    switch_to_state(State::start_state);
                }
                else if (codepoint == '\\')
                {
                    switch_to_state(State::escape_state);
                }
                else
                {
                    this->string->push_back(codepoint);
                }
            }
            break;

        case State::escape_state:
            {
                switch (codepoint)
                {
                case '\"':
                    this->string->push_back(0x0022);
                    switch_to_state(State::string_state);
                    break;

                case '\\':
                    this->string->push_back(0x005C);
                    switch_to_state(State::string_state);
                    break;

                case '/':
                    this->string->push_back(0x002F);
                    switch_to_state(State::string_state);
                    break;

                case 'b':
                    this->string->push_back(0x0008);
                    switch_to_state(State::string_state);
                    break;

                case 'f':
                    this->string->push_back(0x000C);
                    switch_to_state(State::string_state);
                    break;

                case 'n':
                    this->string->push_back(0x000A);
                    switch_to_state(State::string_state);
                    break;

                case 'r':
                    this->string->push_back(0x000D);
                    switch_to_state(State::string_state);
                    break;

                case 't':
                    this->string->push_back(0x0009);
                    switch_to_state(State::string_state);
                    break;

                case 'u':
                    this->hex_number_stream.reset(&this->whole);
                    switch_to_state(State::character_code_state);
                    break;

                default:
                    handle_error("invalid escape character");
                    write_element(codepoint);
                    return;
                }
            }
            break;

        case State::utf16_surrogate_state:
            {
                switch (codepoint)
                {
                case 'u':
                    this->hex_number_stream.reset(&this->whole);
                    switch_to_state(State::character_code_state);
                    break;

                default:
                    handle_error("invalid utf16 surrogate (no low after high)");
                    write_element(codepoint);
                    return;
                }
            }
            break;

        case State::character_code_state:
            {
                bool success = this->hex_number_stream.WriteDigit(codepoint);
                if (!success)
                {
                    if (this->hex_number_stream.get_digit_count() != 4)
                    {
                        handle_error("character code does not have 4 digits");
                    }
                    else if (this->utf16_surrogate)
                    {
                        this->utf16_surrogate = false;

                        Codepoint c16 = static_cast<Codepoint>(this->whole);
                        if (c16 >= 0xDC00 && c16 <= 0xDFFF)
                        {
                            Codepoint c32 = 0x10000 + ((this->utf16_lead_surrogate - 0xD800) << 10) + (c16 - 0xDC00);
                            this->string->push_back(c32);
                            switch_to_state(State::string_state);
                        }
                        else
                        {
                            handle_error("low utf16 surrogate is invalid: !(c >= 0xDC00 && c <= 0xDFFF)");
                        }
                    }
                    else
                    {
                        Codepoint c16 = static_cast<Codepoint>(this->whole);
                        if (c16 >= 0xD800 && c16 <= 0xDBFF)
                        {
                            this->utf16_surrogate = true;
                            this->utf16_lead_surrogate = c16;
                            switch_to_state(State::string_state);
                        }
                        else if (c16 >= 0xDC00 && c16 <= 0xDFFF)
                        {
                            handle_error("character code is invalid: c >= 0xDC00 && c <= 0xDFFF");
                        }
                        else
                        {
                            this->string->push_back(c16);
                            switch_to_state(State::string_state);
                        }
                    }

                    // recurse with the codepoint that was after the hex digit(s)
                    write_element(codepoint);
                    return;
                }
            }
            break;

        case State::number_state:
            {
                bool success = this->dec_number_stream.WriteDigit(codepoint);
                if (!success)
                {
                    if (this->dec_number_stream.get_digit_count() == 0)
                    {
                        handle_error("number token has no digits");
                        write_element(codepoint);
                        return;
                    }
                    else if (codepoint == '.')
                    {
                        this->number = (long double)this->whole * (long double)this->sign;
                        this->dec_number_stream.reset(&this->fraction);
                        switch_to_state(State::fraction_state);
                    }
                    else if (Basic::lower_case(codepoint) == 'e')
                    {
                        this->number = (long double)this->whole * (long double)this->sign;
                        this->dec_number_stream.reset(&this->exponent);
                        switch_to_state(State::exponent_state);
                    }
                    else
                    {
                        this->number = (long double)this->whole * (long double)this->sign;

                        std::shared_ptr<NumberToken> token = std::make_shared<NumberToken>();
                        token->value = this->number;
                        Emit(token);

                        switch_to_state(State::start_state);
                        write_element(codepoint);
                        return;
                    }
                }
            }
            break;

        case State::fraction_state:
            {
                bool success = this->dec_number_stream.WriteDigit(codepoint);
                if (!success)
                {
                    if (this->dec_number_stream.get_digit_count() == 0)
                    {
                        handle_error("fraction part has no digits");
                        write_element(codepoint);
                        return;
                    }
                    else if (Basic::lower_case(codepoint) == 'e')
                    {
                        this->number += this->fraction / (long double)pow((long double)10, (long double)this->dec_number_stream.get_digit_count());
                        this->dec_number_stream.reset(&this->exponent);
                        switch_to_state(State::exponent_state);
                    }
                    else
                    {
                        this->number += this->fraction / (long double)pow((long double)10, (long double)this->dec_number_stream.get_digit_count());

                        std::shared_ptr<NumberToken> token = std::make_shared<NumberToken>();
                        token->value = this->number;
                        Emit(token);

                        switch_to_state(State::start_state);
                        write_element(codepoint);
                        return;
                    }
                }
            }
            break;

        case State::exponent_state:
            {
                bool success = this->dec_number_stream.WriteDigit(codepoint);
                if (!success)
                {
                    if (this->dec_number_stream.get_digit_count() == 0)
                    {
                        handle_error("exponent part has no digits");
                        write_element(codepoint);
                        return;
                    }
                    else
                    {
                        this->number = (long double)pow(this->number, (long double)this->exponent);

                        std::shared_ptr<NumberToken> token = std::make_shared<NumberToken>();
                        token->value = this->number;
                        Emit(token);

                        switch_to_state(State::start_state);
                        write_element(codepoint);
                        return;
                    }
                }
            }
            break;

        default:
            throw FatalError("Json", "Tokenizer", "write_element", "unhandled state", this->get_state());
        }
    }
}