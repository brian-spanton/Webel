// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Json.Types.h"
#include "Json.ValueFrame.h"
#include "Json.ScriptFrame.h"

namespace Json
{
    using namespace Basic;

    class Parser;

    class ObjectFrame : public StateMachine, public UnitStream<std::shared_ptr<Token> >
    {
    private:
        enum State
        {
            expecting_first_name_state = Start_State,
            script_frame_pending_state,
            expecting_next_name_state,
            expecting_separator_state,
            member_value_frame_pending_state,
            expecting_member_separator_state,
            done_state = Succeeded_State,
            script_frame_failed,
            expecting_first_name_error,
            expecting_separator_error,
            member_value_frame_failed,
            expecting_member_separator_error,
            expecting_next_name_error,
        };

        Object* value;
        UnicodeStringRef member_name;
        std::shared_ptr<Value> member_value;
        std::shared_ptr<ValueFrame> member_value_frame;
        std::shared_ptr<Script> script;
        std::shared_ptr<ScriptFrame> script_frame;
        std::shared_ptr<Html::Node> domain;
        std::shared_ptr<Html::Node> element_domain;
        State return_to = State::expecting_first_name_state;

    public:
        ObjectFrame(std::shared_ptr<Html::Node> domain, Object* value);

        virtual void IStream<std::shared_ptr<Token> >::write_element(std::shared_ptr<Token> element);
    };
}