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

	class ObjectFrame : public Frame
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
		UnicodeString::Ref member_name; // REF
		Value::Ref member_value; // REF
		Inline<ValueFrame> member_value_frame;
		Inline<ScriptFrame> script_frame;
		Html::Node::Ref domain; // REF
		Html::Node::Ref element_domain; // REF
		Script script;
		State return_to;

	public:
		typedef Basic::Ref<ObjectFrame, IProcess> Ref;

		void Initialize(Html::Node::Ref domain, Object* value);

		virtual void IProcess::Process(IEvent* event, bool* yield);
	};
}