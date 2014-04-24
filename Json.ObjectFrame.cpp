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

    void ObjectFrame::Initialize(Html::Node::Ref domain, Object* value)
    {
        __super::Initialize();

        this->domain = domain;
        this->element_domain = domain;
        this->value = value;
    }

    void ObjectFrame::Process(IEvent* event, bool* yield)
    {
        switch (frame_state())
        {
        case State::expecting_first_name_state:
            {
                Token::Ref token;
                if (!ReadyForReadTokenPointerEvent::ReadNext(event, &token, yield))
                    return;

                switch(token->type)
                {
                case Token::Type::begin_script_token:
                    this->script_frame.Initialize(this->domain, &this->script);
                    this->return_to = State::expecting_first_name_state;
                    switch_to_state(State::script_frame_pending_state);
                    break;

                case Token::Type::end_object_token:
                    switch_to_state(State::done_state);
                    break;

                case Token::Type::string_token:
                    this->member_name = ((StringToken*)token.item())->value;
                    switch_to_state(State::expecting_separator_state);
                    break;

                default:
                    ReadyForReadTokenPointerEvent::UndoReadNext(event);
                    switch_to_state(State::expecting_first_name_error);
                    break;
                }
            }
            break;

        case State::script_frame_pending_state:
            if (this->script_frame.Pending())
            {
                this->script_frame.Process(event, yield);
            }
            
            if (this->script_frame.Failed())
            {
                switch_to_state(State::script_frame_failed);
            }
            else if (this->script_frame.Succeeded())
            {
                Html::Node::Ref element;

                bool success = this->script.Execute(this->domain, 0, &element);
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
                Token::Ref token;
                if (!ReadyForReadTokenPointerEvent::ReadNext(event, &token, yield))
                    return;

                switch(token->type)
                {
                case Token::Type::name_separator_token:
                    this->member_value_frame.Initialize(this->element_domain, &this->member_value);
                    switch_to_state(State::member_value_frame_pending_state);
                    break;

                default:
                    ReadyForReadTokenPointerEvent::UndoReadNext(event);
                    switch_to_state(State::expecting_separator_error);
                    break;
                }
            }
            break;

        case State::member_value_frame_pending_state:
            if (this->member_value_frame.Pending())
            {
                this->member_value_frame.Process(event, yield);
            }

            if (this->member_value_frame.Failed())
            {
                switch_to_state(State::member_value_frame_failed);
            }
            else if (this->member_value_frame.Succeeded())
            {
                this->value->members.insert(MemberList::value_type(this->member_name, this->member_value));
                switch_to_state(State::expecting_member_separator_state);
            }
            break;

        case State::expecting_member_separator_state:
            {
                Token::Ref token;
                if (!ReadyForReadTokenPointerEvent::ReadNext(event, &token, yield))
                    return;

                switch(token->type)
                {
                case Token::Type::end_object_token:
                    switch_to_state(State::done_state);
                    break;

                case Token::Type::value_separator_token:
                    switch_to_state(State::expecting_next_name_state);
                    break;

                default:
                    ReadyForReadTokenPointerEvent::UndoReadNext(event);
                    switch_to_state(State::expecting_member_separator_error);
                    break;
                }
            }
            break;

        case State::expecting_next_name_state:
            {
                Token::Ref token;
                if (!ReadyForReadTokenPointerEvent::ReadNext(event, &token, yield))
                    return;

                switch(token->type)
                {
                case Token::Type::begin_script_token:
                    this->script_frame.Initialize(this->domain, &this->script);
                    this->return_to = State::expecting_next_name_state;
                    switch_to_state(State::script_frame_pending_state);
                    break;

                case Token::Type::string_token:
                    this->member_name = ((StringToken*)token.item())->value;
                    switch_to_state(State::expecting_separator_state);
                    break;

                default:
                    ReadyForReadTokenPointerEvent::UndoReadNext(event);
                    switch_to_state(State::expecting_next_name_error);
                    break;
                }
            }
            break;

        default:
            throw new Exception("Json::ObjectFrame::Process unexpected state");
        }
    }
}