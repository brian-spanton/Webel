// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Json.Types.h"
#include "Json.ScriptFrame.h"

namespace Json
{
    using namespace Basic;

    class Parser;
    class ArrayFrame;
    class ObjectFrame;

    class ValueFrame : public StateMachine, public UnitStream<std::shared_ptr<Token> >
    {
    private:
        enum State
        {
            start_state = Start_State,
            script_frame_pending_state,
            array_frame_pending_state,
            object_frame_pending_state,
            done_state = Succeeded_State,
            script_frame_failed,
            start_state_error,
            array_frame_failed,
            object_frame_failed,
        };

        std::shared_ptr<Value>* value;
        std::shared_ptr<Array> array;
        std::shared_ptr<ArrayFrame> array_frame;
        std::shared_ptr<Object> object;
        std::shared_ptr<ObjectFrame> object_frame;
        std::shared_ptr<Script> script;
        std::shared_ptr<ScriptFrame> script_frame;
        std::shared_ptr<Html::Node> domain;

    public:
        ValueFrame(std::shared_ptr<Html::Node> domain, std::shared_ptr<Value>* value);

        void reset();

        virtual void IStream<std::shared_ptr<Token> >::write_element(std::shared_ptr<Token> element);
    };
}