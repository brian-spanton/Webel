// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Json.ArrayFrame.h"
#include "Json.Globals.h"
#include "Json.Types.h"
#include "Json.Parser.h"
#include "Json.ValueFrame.h"

namespace Json
{
    using namespace Basic;

    ArrayFrame::ArrayFrame(std::shared_ptr<Html::Node> domain, Array* value) :
        domain(domain),
        element_domain(domain),
        value(value)
    {
    }

    bool ArrayFrame::FindNextScriptElement()
    {
        std::shared_ptr<Html::Node> script_element;

        bool success = this->script->Execute(this->domain, this->start_from, &script_element);
        if (!success)
        {
            this->start_from = 0;
            this->element_domain = this->domain;
            return false;
        }

        this->start_from = script_element;
        this->element_domain = script_element;
        return true;
    }

    void ArrayFrame::write_element(std::shared_ptr<Token> token)
    {
        if (this->scripted_tokens.get() != 0)
        {
            this->scripted_tokens->push_back(token);
        }

        WriteUnobserved(token);
    }

    void ArrayFrame::WriteUnobserved(std::shared_ptr<Token> token)
    {
        switch (get_state())
        {
        case State::expecting_first_element_state:
            {
                switch (token->type)
                {
                case Token::Type::begin_script_token:
                    this->script = std::make_shared<Script>();
                    this->script_frame = std::make_shared<ScriptFrame>(this->domain, this->script.get());
                    switch_to_state(State::script_frame_pending_state);
                    break;

                case Token::Type::end_array_token:
                    switch_to_state(State::done_state);
                    break;

                default:
                    this->element_frame = std::make_shared<ValueFrame>(this->element_domain, &this->element);
                    switch_to_state(State::element_frame_pending_state);
                    WriteUnobserved(token);
                    return;
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

                bool success = FindNextScriptElement();
                if (success)
                {
                    this->scripted_tokens = std::make_shared<TokenVector>();
                }

                this->element_frame = std::make_shared<ValueFrame>(this->element_domain, &this->element);
                switch_to_state(State::script_execution_state);
            }
            break;

        case State::script_execution_state:
            {
                this->element_frame->write_element(token);

                if (this->element_frame->in_progress())
                    return;

                if (this->element_frame->failed())
                {
                    switch_to_state(State::element_frame_failed);
                    return;
                }

                if (this->scripted_tokens.get() != 0)
                {
                    this->value->elements.push_back(this->element);

                    std::shared_ptr<TokenVector> scripted_tokens;
                    scripted_tokens.swap(this->scripted_tokens);

                    while (true)
                    {
                        bool success = FindNextScriptElement();
                        if (!success)
                            break;

                        this->element_frame = std::make_shared<ValueFrame>(this->element_domain, &this->element);

                        for (TokenVector::iterator it = scripted_tokens->begin(); it != scripted_tokens->end(); it++)
                        {
                            this->element_frame->write_element(*it);

                            if (this->element_frame->in_progress())
                                continue;
                        }

                        if (this->element_frame->failed())
                        {
                            switch_to_state(State::element_frame_failed);
                            return;
                        }

                        this->value->elements.push_back(this->element);
                    }
                }

                switch_to_state(State::expecting_value_separator_state);
            }
            break;

        case State::element_frame_pending_state:
            {
                this->element_frame->write_element(token);

                if (this->element_frame->in_progress())
                    return;

                if (this->element_frame->failed())
                {
                    switch_to_state(State::element_frame_failed);
                    return;
                }

                this->value->elements.push_back(this->element);
                switch_to_state(State::expecting_value_separator_state);
            }
            break;

        case State::expecting_value_separator_state:
            {
                switch (token->type)
                {
                case Token::Type::end_array_token:
                    switch_to_state(State::done_state);
                    break;

                case Token::Type::value_separator_token:
                    switch_to_state(State::expecting_next_element_state);
                    break;

                default:
                    switch_to_state(State::expecting_value_separator_error);
                    break;
                }
            }
            break;

        case State::expecting_next_element_state:
            {
                switch (token->type)
                {
                case Token::Type::begin_script_token:
                    this->script = std::make_shared<Script>();
                    this->script_frame = std::make_shared<ScriptFrame>(this->domain, this->script.get());
                    switch_to_state(State::script_frame_pending_state);
                    break;

                default:
                    this->element_frame = std::make_shared<ValueFrame>(this->element_domain, &this->element);
                    switch_to_state(State::element_frame_pending_state);
                    WriteUnobserved(token);
                    return;
                }
            }
            break;

        default:
            throw FatalError("Json::ArrayFrame::handle_event unexpected state");
        }
    }
}