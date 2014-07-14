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

    class ArrayFrame : public StateMachine, public UnitStream<std::shared_ptr<Token> >
    {
    private:
        enum State
        {
            expecting_first_element_state = Start_State,
            script_frame_pending_state,
            script_execution_state,
            element_frame_pending_state,
            expecting_value_separator_state,
            expecting_next_element_state,
            done_state = Succeeded_State,
            script_frame_failed,
            element_frame_failed,
            expecting_value_separator_error,
        };

        Array* value;
        std::shared_ptr<Value> element;
        std::shared_ptr<ValueFrame> element_frame;
        std::shared_ptr<Script> script;
        std::shared_ptr<ScriptFrame> script_frame;
        std::shared_ptr<Html::Node> domain;
        std::shared_ptr<Html::Node> element_domain;
        std::shared_ptr<Html::Node> start_from;
        std::shared_ptr<TokenVector> scripted_tokens;

        bool FindNextScriptElement();
        void WriteUnobserved(std::shared_ptr<Token> element);

    public:
        ArrayFrame(std::shared_ptr<Html::Node> domain, Array* value);

        virtual void IStream<std::shared_ptr<Token> >::write_element(std::shared_ptr<Token> element);
    };
}