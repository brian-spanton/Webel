// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Json.Text.h"
#include "Json.Globals.h"
#include "Json.Types.h"
#include "Json.Tokenizer.h"
#include "Json.Parser.h"
#include "Json.ObjectFrame.h"
#include "Json.ArrayFrame.h"
#include "Basic.Event.h"

namespace Json
{
	using namespace Basic;

	void Text::Initialize(Html::Node::Ref domain)
	{
		__super::Initialize();

		this->domain = domain;
	}

	void Text::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::expecting_root_state:
			{
				Token::Ref token;
				if (!ReadyForReadTokenPointerEvent::ReadNext(event, &token, yield))
					return;

				switch(token->type)
				{
				case Token::Type::begin_script_token:
					{
						this->script_frame.Initialize(this->domain, &this->script);
						switch_to_state(State::script_frame_pending_state);
					}
					break;

				case Token::Type::begin_array_token:
					{
						Array::Ref value = New<Array>();
						this->value = value;

						this->array_frame.Initialize(this->domain, value);
						switch_to_state(State::array_frame_pending_state);
					}
					break;

				case Token::Type::begin_object_token:
					{
						Object::Ref value = New<Object>();
						this->value = value;

						this->object_frame.Initialize(this->domain, value);
						switch_to_state(State::object_frame_pending_state);
					}
					break;

				default:
					ReadyForReadTokenPointerEvent::UndoReadNext(event);
					switch_to_state(State::expecting_root_error);
					break;
				}
			}
			break;

		case State::script_frame_pending_state:
			if (this->script_frame.Pending())
			{
				this->script_frame.Process(event, yield);
			}
			else if (this->script_frame.Failed())
			{
				switch_to_state(State::script_frame_failed);
			}
			else
			{
				Html::Node::Ref element;

				bool success = this->script.Execute(this->domain, 0, &element);
				if (success)
				{
					this->domain = element;
				}

				switch_to_state(State::expecting_root_state);
			}
			break;

		case State::array_frame_pending_state:
			if (this->array_frame.Pending())
			{
				this->array_frame.Process(event, yield);
			}
			else if (this->array_frame.Failed())
			{
				switch_to_state(State::array_frame_failed);
			}
			else
			{
				switch_to_state(State::done_state);
			}
			break;

		case State::object_frame_pending_state:
			if (this->object_frame.Pending())
			{
				this->object_frame.Process(event, yield);
			}
			else if (this->object_frame.Failed())
			{
				switch_to_state(State::object_frame_failed);
			}
			else
			{
				switch_to_state(State::done_state);
			}
			break;

		default:
			throw new Exception("Json::Text::Process unexpected state");
		}
	}
}