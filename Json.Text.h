// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Json.Types.h"
#include "Json.ArrayFrame.h"
#include "Json.ObjectFrame.h"
#include "Json.ScriptFrame.h"

namespace Json
{
    using namespace Basic;

    class Parser;

    class Text : public StateMachine, public UnitStream<std::shared_ptr<Token> >
    {
    private:
        enum State
        {
            expecting_root_state = Start_State,
            script_frame_pending_state,
            array_frame_pending_state,
            object_frame_pending_state,
            done_state = Succeeded_State,
            expecting_root_error,
            script_frame_failed,
            array_frame_failed,
            object_frame_failed,
        };

        std::shared_ptr<ArrayFrame> array_frame;
        std::shared_ptr<ObjectFrame> object_frame;
        std::shared_ptr<Script> script;
        std::shared_ptr<ScriptFrame> script_frame;
        std::shared_ptr<Html::Node> domain;

    public:
        std::shared_ptr<Value> value;

        Text(std::shared_ptr<Html::Node> domain);

        virtual void IStream<std::shared_ptr<Token> >::write_element(std::shared_ptr<Token> element);
    };
}