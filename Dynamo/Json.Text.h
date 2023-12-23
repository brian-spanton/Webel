#pragma once

#include "Basic.IStream.h"
#include "Json.Types.h"
#include "Json.ArrayFrame.h"
#include "Json.ObjectFrame.h"
#include "Html.Node.h"
#include "Json.ScriptFrame.h"

namespace Json
{
	using namespace Basic;

	class Parser;

	class Text : public Frame
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

		Inline<ArrayFrame> array_frame;
		Inline<ObjectFrame> object_frame;
		Inline<ScriptFrame> script_frame;
		Html::Node::Ref domain; // $$$
		Script script;

	public:
		typedef Basic::Ref<Text, IProcess> Ref;

		Value::Ref value; // $$$

		void Initialize(Html::Node::Ref domain);

		virtual void IProcess::Process(IEvent* event, bool* yield);
	};
}