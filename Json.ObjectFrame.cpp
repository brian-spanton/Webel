// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Json.ObjectFrame.h"
#include "Json.Globals.h"
#include "Json.Types.h"
#include "Json.Tokenizer.h"
#include "Json.Parser.h"
#include "Json.ValueFrame.h"

namespace Json
{
    using namespace Basic;

    ObjectFrame::ObjectFrame(std::shared_ptr<Html::Node> domain, Object* value) :
        domain(domain),
        element_domain(this->domain), // initialization is in order of declaration in class def
        value(value)
    {
    }

    void ObjectFrame::write_element(std::shared_ptr<Token> token)
    {
        switch (get_state())
        {
        case State::expecting_first_name_state:
            {
                switch(token->type)
                {
                case Token::Type::begin_script_token:
                    this->script = std::make_shared<Script>();
                    this->script_frame = std::make_shared<ScriptFrame>(this->domain, this->script.get());
                    this->return_to = State::expecting_first_name_state;
                    switch_to_state(State::script_frame_pending_state);
                    break;

                case Token::Type::end_object_token:
                    switch_to_state(State::done_state);
                    break;

                case Token::Type::string_token:
                    this->member_name = ((StringToken*)token.get())->value;
                    switch_to_state(State::expecting_separator_state);
                    break;

                default:
                    switch_to_state(State::expecting_first_name_error);
                    break;
                }
            }
            break;

        case State::script_frame_pending_state:
            {
                this->script_frame->write_element(token);

                if (this->script_frame->in_progress())
                    return;

                if (this->script_frame->failed())
                {
                    switch_to_state(State::script_frame_failed);
                    return;
                }

                std::shared_ptr<Html::Node> element;

                bool success = this->script->Execute(this->domain, 0, &element);
                if (success)
                {
                    this->element_domain = element;
                }
                else
                {
                    this->element_domain = this->domain;
                }

                switch_to_state(this->return_to);
            }
            break;

        case State::expecting_separator_state:
            {
                switch(token->type)
                {
                case Token::Type::name_separator_token:
                    this->member_value_frame = std::make_shared<ValueFrame>(this->element_domain, &this->member_value);
                    switch_to_state(State::member_value_frame_pending_state);
                    break;

                default:
                    switch_to_state(State::expecting_separator_error);
                    break;
                }
            }
            break;

        case State::member_value_frame_pending_state:
            {
                this->member_value_frame->write_element(token);

                if (this->member_value_frame->in_progress())
                    return;

                if (this->member_value_frame->failed())
                {
                    switch_to_state(State::member_value_frame_failed);
                    return;
                }

                this->value->members.insert(MemberList::value_type(this->member_name, this->member_value));
                switch_to_state(State::expecting_member_separator_state);
            }
            break;

        case State::expecting_member_separator_state:
            {
                switch(token->type)
                {
                case Token::Type::end_object_token:
                    switch_to_state(State::done_state);
                    break;

                case Token::Type::value_separator_token:
                    switch_to_state(State::expecting_next_name_state);
                    break;

                default:
                    switch_to_state(State::expecting_member_separator_error);
                    break;
                }
            }
            break;

        case State::expecting_next_name_state:
            {
                switch(token->type)
                {
                case Token::Type::begin_script_token:
                    this->script = std::make_shared<Script>();
                    this->script_frame = std::make_shared<ScriptFrame>(this->domain, this->script.get());
                    this->return_to = State::expecting_next_name_state;
                    switch_to_state(State::script_frame_pending_state);
                    break;

                case Token::Type::string_token:
                    this->member_name = ((StringToken*)token.get())->value;
                    switch_to_state(State::expecting_separator_state);
                    break;

                default:
                    switch_to_state(State::expecting_next_name_error);
                    break;
                }
            }
            break;

        default:
            throw FatalError("Json", "ObjectFrame::write_element unhandled state");
        }
    }
}