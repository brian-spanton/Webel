// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Json.ScriptFrame.h"
#include "Json.Globals.h"
#include "Json.Types.h"
#include "Json.Tokenizer.h"
#include "Json.Parser.h"
#include "Json.ValueFrame.h"

namespace Json
{
    using namespace Basic;

    void ScriptFrame::Initialize(Html::Node::Ref domain, Json::Script* value)
    {
        __super::Initialize();

        this->domain = domain;
        this->value = value;
        this->value->Initialize();
    }

    void ScriptFrame::ParseError(const char* error)
    {
        HandleError(error);
    }

    void ScriptFrame::Process(IEvent* event, bool* yield)
    {
        switch (frame_state())
        {
        case State::expecting_element_state:
            {
                Token::Ref token;
                if (!ReadyForReadTokenPointerEvent::ReadNext(event, &token, yield))
                    return;

                switch(token->type)
                {
                case Token::Type::token_token:
                    this->value->element_name = ((TokenToken*)token.item())->value;
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
                Token::Ref token;
                if (!ReadyForReadTokenPointerEvent::ReadNext(event, &token, yield))
                    return;

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
                    this->value->element_name = (UnicodeString*)0;
                    this->parameter_value_frame = New<ValueFrame>();
                    this->parameter_value_frame->Initialize(this->domain, &this->value->parameter_value);
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
                Token::Ref token;
                if (!ReadyForReadTokenPointerEvent::ReadNext(event, &token, yield))
                    return;

                switch(token->type)
                {
                case Token::Type::token_token:
                    this->value->attribute_name = ((TokenToken*)token.item())->value;
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
                Token::Ref token;
                if (!ReadyForReadTokenPointerEvent::ReadNext(event, &token, yield))
                    return;

                switch(token->type)
                {
                case Token::Type::token_separator_token:
                    switch_to_state(State::expecting_method_state);
                    break;

                case Token::Type::begin_parameter_token:
                    this->value->method_name = this->value->attribute_name;
                    this->value->attribute_name = (UnicodeString*)0;
                    this->parameter_value_frame = New<ValueFrame>();
                    this->parameter_value_frame->Initialize(this->domain, &this->value->parameter_value);
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
                Token::Ref token;
                if (!ReadyForReadTokenPointerEvent::ReadNext(event, &token, yield))
                    return;

                switch(token->type)
                {
                case Token::Type::token_token:
                    this->value->method_name = ((TokenToken*)token.item())->value;
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
                Token::Ref token;
                if (!ReadyForReadTokenPointerEvent::ReadNext(event, &token, yield))
                    return;

                switch(token->type)
                {
                case Token::Type::begin_parameter_token:
                    this->parameter_value_frame = New<ValueFrame>();
                    this->parameter_value_frame->Initialize(this->domain, &this->value->parameter_value);
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
                Token::Ref token;
                if (!ReadyForReadTokenPointerEvent::ReadNext(event, &token, yield))
                    return;

                switch(token->type)
                {
                case Token::Type::end_parameter_token:
                    switch_to_state(State::expecting_end_script_state);
                    break;

                default:
                    ReadyForReadTokenPointerEvent::UndoReadNext(event);
                    switch_to_state(State::expecting_parameter_state);
                    break;
                }
            }
            break;

        case State::expecting_parameter_state:
            if (this->parameter_value_frame->Pending())
            {
                this->parameter_value_frame->Process(event, yield);
            }

            if (this->parameter_value_frame->Failed())
            {
                ParseError("parameter_value_frame failed");
                switch_to_state(State::parse_error);
            }
            else if (this->parameter_value_frame->Succeeded())
            {
                switch_to_state(State::expecting_end_parameter_state);
            }
            break;

        case State::expecting_end_parameter_state:
            {
                Token::Ref token;
                if (!ReadyForReadTokenPointerEvent::ReadNext(event, &token, yield))
                    return;

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
                Token::Ref token;
                if (!ReadyForReadTokenPointerEvent::ReadNext(event, &token, yield))
                    return;

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
            throw new Exception("Json::ScriptFrame::Process unexpected state");
        }
    }
}