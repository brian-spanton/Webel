// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Json.ValueFrame.h"
#include "Json.Globals.h"
#include "Json.Types.h"
#include "Json.Parser.h"
#include "Json.ObjectFrame.h"
#include "Json.ArrayFrame.h"
#include "Basic.Event.h"

namespace Json
{
    using namespace Basic;

    ValueFrame::ValueFrame(std::shared_ptr<Html::Node> domain, std::shared_ptr<Value>* value) :
        domain(domain),
        value(value)
    {
    }

    void ValueFrame::reset()
    {
        __super::reset();
    }

    void ValueFrame::write_element(std::shared_ptr<Token> token)
    {
        switch (get_state())
        {
        case State::start_state:
            {
                switch(token->type)
                {
                case Token::Type::begin_script_token:
                    this->script = std::make_shared<Script>();
                    this->script_frame = std::make_shared<ScriptFrame>(this->domain, this->script.get());
                    switch_to_state(State::script_frame_pending_state);
                    break;

                case Token::Type::begin_array_token:
                    {
                        std::shared_ptr<Array> value = std::make_shared<Array>();
                        (*this->value) = value;

                        this->array_frame = std::make_shared<ArrayFrame>(this->domain, value.get());
                        switch_to_state(State::array_frame_pending_state);
                    }
                    break;

                case Token::Type::begin_object_token:
                    {
                        std::shared_ptr<Object> value = std::make_shared<Object>();
                        (*this->value) = value;

                        this->object_frame = std::make_shared<ObjectFrame>(this->domain, value.get());
                        switch_to_state(State::object_frame_pending_state);
                    }
                    break;

                case Token::Type::number_token:
                    {
                        std::shared_ptr<Number> value = std::make_shared<Number>();
                        (*this->value) = value;

                        value->value = ((NumberToken*)token.get())->value;

                        switch_to_state(State::done_state);
                    }
                    break;

                case Token::Type::string_token:
                    {
                        std::shared_ptr<String> value = std::make_shared<String>();
                        (*this->value) = value;

                        value->value = ((StringToken*)token.get())->value;

                        switch_to_state(State::done_state);
                    }
                    break;

                case Token::Type::bool_token:
                    {
                        std::shared_ptr<Bool> value = std::make_shared<Bool>();
                        (*this->value) = value;

                        value->value = ((BoolToken*)token.get())->value;

                        switch_to_state(State::done_state);
                    }
                    break;

                case Token::Type::null_token:
                    {
                        std::shared_ptr<Null> value = std::make_shared<Null>();
                        (*this->value) = value;

                        switch_to_state(State::done_state);
                    }
                    break;

                default:
                    switch_to_state(State::start_state_error);
                    break;
                }
            }
            break;

        case State::script_frame_pending_state:
            {
                this->script_frame->write_element(token);

                if (this->script_frame->in_progress())
                    break;

                if (this->script_frame->failed())
                {
                    switch_to_state(State::script_frame_failed);
                    break;
                }

                UnicodeStringRef element;

                bool success = this->script->Execute(this->domain, &element);
                if (success)
                {
                    std::shared_ptr<String> value = std::make_shared<String>();
                    (*this->value) = value;

                    value->value = element;
                }
                else
                {
                    std::shared_ptr<Null> value = std::make_shared<Null>();
                    (*this->value) = value;
                }

                switch_to_state(State::done_state);
            }
            break;

        case State::array_frame_pending_state:
            {
                this->array_frame->write_element(token);

                if (this->array_frame->in_progress())
                    return;

                if (this->array_frame->failed())
                {
                    switch_to_state(State::array_frame_failed);
                    return;
                }

                switch_to_state(State::done_state);
            }
            break;

        case State::object_frame_pending_state:
            {
                this->object_frame->write_element(token);

                if (this->object_frame->in_progress())
                    return;

                if (this->object_frame->failed())
                {
                    switch_to_state(State::object_frame_failed);
                    return;
                }

                switch_to_state(State::done_state);
            }
            break;

        default:
            throw FatalError("Json", "ValueFrame::write_element unhandled state");
        }
    }
}