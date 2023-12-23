#pragma once

#include "Basic.IStream.h"
#include "Basic.Frame.h"
#include "Json.Types.h"
#include "Json.ScriptFrame.h"

namespace Json
{
	using namespace Basic;

	class Parser;
	class ArrayFrame;
	class ObjectFrame;

	class ValueFrame : public Frame
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

		Value::Ref* value; // $$$
		Basic::Ref<ArrayFrame> array_frame; // $$$
		Basic::Ref<ObjectFrame> object_frame; // $$$
		Inline<ScriptFrame> script_frame;
		Html::Node::Ref domain; // $$$
		Script script;

	public:
		typedef Basic::Ref<ValueFrame, IProcess> Ref;

		void Initialize(Html::Node::Ref domain, Value::Ref* value);

		virtual void IProcess::Process(IEvent* event, bool* yield);
	};
}