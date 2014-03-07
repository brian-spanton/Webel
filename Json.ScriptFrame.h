// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"
#include "Json.Script.h"

namespace Json
{
	using namespace Basic;

	class Parser;
	class ValueFrame;

	class ScriptFrame : public Frame
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
		Basic::Ref<ValueFrame> parameter_value_frame; // REF
		Html::Node::Ref domain; // REF

		void ParseError(const char* error);

	public:
		typedef Basic::Ref<ScriptFrame, IProcess> Ref;

		void Initialize(Html::Node::Ref domain, Script* value);

		virtual void IProcess::Process(IEvent* event, bool* yield);
	};
}