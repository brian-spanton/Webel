// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Json.Text.h"
#include "Json.Globals.h"
#include "Json.Types.h"
#include "Json.Parser.h"
#include "Json.ObjectFrame.h"
#include "Json.ArrayFrame.h"
#include "Basic.Event.h"

namespace Json
{
    using namespace Basic;

    Text::Text(std::shared_ptr<Html::Node> domain) :
        domain(domain)
    {
    }

    void Text::write_element(std::shared_ptr<Token> token)
    {
        switch (get_state())
        {
        case State::expecting_root_state:
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
                        std::shared_ptr<Array> array_value = std::make_shared<Array>();
                        this->array_frame = std::make_shared<ArrayFrame>(this->domain, array_value.get());
                        this->value = array_value;
                        switch_to_state(State::array_frame_pending_state);
                    }
                    break;

                case Token::Type::begin_object_token:
                    {
                        std::shared_ptr<Object> object_value = std::make_shared<Object>();
                        this->object_frame = std::make_shared<ObjectFrame>(this->domain, object_value.get());
                        this->value = object_value;
                        switch_to_state(State::object_frame_pending_state);
                    }
                    break;

                default:
                    switch_to_state(State::expecting_root_error);
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
                    this->domain = element;
                }

                switch_to_state(State::expecting_root_state);
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
            throw FatalError("Json", "Text::write_element unhandled state");
        }
    }
}