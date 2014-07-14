// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Json.ScriptFrame.h"
#include "Json.Globals.h"
#include "Json.Types.h"
#include "Json.Parser.h"
#include "Json.ValueFrame.h"

namespace Json
{
    using namespace Basic;

    ScriptFrame::ScriptFrame(std::shared_ptr<Html::Node> domain, Json::Script* value) :
        domain(domain),
        value(value)
    {
    }

    void ScriptFrame::ParseError(const char* error)
    {
        HandleError(error);
    }

    void ScriptFrame::write_element(std::shared_ptr<Token> token)
    {
        switch (get_state())
        {
        case State::expecting_element_state:
            {
                switch(token->type)
                {
                case Token::Type::token_token:
                    this->value->element_name = ((TokenToken*)token.get())->value;
                    switch_to_state(State::after_element_state);
                    break;

                case Token::Type::token_separator_token:
                    switch_to_state(State::expecting_attribute_state);
                    break;

                case Token::Type::end_script_token:
                    switch_to_state(State::done_state);
                    break;

                default:
                    ParseError("unexpected token");
                    switch_to_state(State::parse_error);
                    break;
                }
            }
            break;

        case State::after_element_state:
            {
                switch(token->type)
                {
                case Token::Type::token_separator_token:
                    switch_to_state(State::expecting_attribute_state);
                    break;

                case Token::Type::end_script_token:
                    switch_to_state(State::done_state);
                    break;

                case Token::Type::begin_parameter_token:
                    this->value->method_name = this->value->element_name;
                    this->value->element_name = 0;
                    this->parameter_value_frame = std::make_shared<ValueFrame>(this->domain, &this->value->parameter_value);
                    switch_to_state(State::after_begin_parameter_state);
                    break;

                default:
                    ParseError("unexpected token");
                    switch_to_state(State::parse_error);
                    break;
                }
            }
            break;

        case State::expecting_attribute_state:
            {
                switch(token->type)
                {
                case Token::Type::token_token:
                    this->value->attribute_name = ((TokenToken*)token.get())->value;
                    switch_to_state(State::after_attribute_state);
                    break;

                default:
                    ParseError("unexpected token");
                    switch_to_state(State::parse_error);
                    break;
                }
            }
            break;

        case State::after_attribute_state:
            {
                switch(token->type)
                {
                case Token::Type::token_separator_token:
                    switch_to_state(State::expecting_method_state);
                    break;

                case Token::Type::begin_parameter_token:
                    this->value->method_name = this->value->attribute_name;
                    this->value->attribute_name = 0;
                    this->parameter_value_frame = std::make_shared<ValueFrame>(this->domain, &this->value->parameter_value);
                    switch_to_state(State::after_begin_parameter_state);
                    break;

                case Token::Type::end_script_token:
                    switch_to_state(State::done_state);
                    break;

                default:
                    ParseError("unexpected token");
                    switch_to_state(State::parse_error);
                    break;
                }
            }
            break;

        case State::expecting_method_state:
            {
                switch(token->type)
                {
                case Token::Type::token_token:
                    this->value->method_name = ((TokenToken*)token.get())->value;
                    switch_to_state(State::expecting_begin_parameter_state);
                    break;

                default:
                    ParseError("unexpected token");
                    switch_to_state(State::parse_error);
                    break;
                }
            }
            break;

        case State::expecting_begin_parameter_state:
            {
                switch(token->type)
                {
                case Token::Type::begin_parameter_token:
                    this->parameter_value_frame = std::make_shared<ValueFrame>(this->domain, &this->value->parameter_value);
                    switch_to_state(State::after_begin_parameter_state);
                    break;

                default:
                    ParseError("unexpected token");
                    switch_to_state(State::parse_error);
                    break;
                }
            }
            break;

        case State::after_begin_parameter_state:
            {
                switch(token->type)
                {
                case Token::Type::end_parameter_token:
                    switch_to_state(State::expecting_end_script_state);
                    break;

                default:
                    switch_to_state(State::expecting_parameter_state);
                    write_element(token);
                    return;
                }
            }
            break;

        case State::expecting_parameter_state:
            {
                this->parameter_value_frame->write_element(token);

                if (this->parameter_value_frame->in_progress())
                    return;

                if (this->parameter_value_frame->failed())
                {
                    ParseError("parameter_value_frame failed");
                    switch_to_state(State::parse_error);
                    return;
                }

                switch_to_state(State::expecting_end_parameter_state);
            }
            break;

        case State::expecting_end_parameter_state:
            {
                switch(token->type)
                {
                case Token::Type::end_parameter_token:
                    switch_to_state(State::expecting_end_script_state);
                    break;

                default:
                    ParseError("unexpected token");
                    switch_to_state(State::parse_error);
                    break;
                }
            }
            break;

        case State::expecting_end_script_state:
            {
                switch(token->type)
                {
                case Token::Type::end_script_token:
                    switch_to_state(State::done_state);
                    break;

                default:
                    ParseError("unexpected token");
                    switch_to_state(State::parse_error);
                    break;
                }
            }
            break;

        default:
            throw FatalError("Json::ScriptFrame::handle_event unexpected state");
        }
    }
}