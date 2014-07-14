// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"
#include "Json.Script.h"

namespace Json
{
    using namespace Basic;

    class Parser;
    class ValueFrame;

    class ScriptFrame : public StateMachine, public UnitStream<std::shared_ptr<Token> >
    {
    private:
        enum State
        {
            expecting_element_state = Start_State,
            after_element_state,
            expecting_attribute_state,
            after_attribute_state,
            expecting_method_state,
            expecting_begin_parameter_state,
            after_begin_parameter_state,
            expecting_parameter_state,
            expecting_end_parameter_state,
            expecting_end_script_state,
            done_state = Succeeded_State,
            parse_error,
        };

        Script* value;
        std::shared_ptr<ValueFrame> parameter_value_frame;
        std::shared_ptr<Html::Node> domain;

        void ParseError(const char* error);

    public:
        ScriptFrame(std::shared_ptr<Html::Node> domain, Script* value);

        virtual void IStream<std::shared_ptr<Token> >::write_element(std::shared_ptr<Token> element);
    };
}