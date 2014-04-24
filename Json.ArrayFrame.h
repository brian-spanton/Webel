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

    class ArrayFrame : public Frame
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
        Value::Ref element; // REF
        Inline<ValueFrame> element_frame;
        Inline<ScriptFrame> script_frame;
        Html::Node::Ref domain; // REF
        Html::Node::Ref element_domain; // REF
        Html::Node::Ref start_from; // REF
        Script script;
        TokenVector::Ref scripted_tokens; // REF
        Basic::Ref<IElementSource<Token::Ref> > element_source; // REF

        bool FindNextScriptElement();

    public:
        typedef Basic::Ref<ArrayFrame, IProcess> Ref;

        void Initialize(Html::Node::Ref domain, Array* value);

        virtual void IProcess::Process(IEvent* event, bool* yield);
    };
}