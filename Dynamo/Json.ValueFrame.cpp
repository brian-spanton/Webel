#include "stdafx.h"
#include "Json.ValueFrame.h"
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

	void ValueFrame::Initialize(Html::Node::Ref domain, Value::Ref* value)
	{
		__super::Initialize();

		this->domain = domain;
		this->value = value;
	}

	void ValueFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::start_state:
			{
				Token::Ref token;
				if (!ReadyForReadTokenPointerEvent::ReadNext(event, &token, yield))
					return;

				switch(token->type)
				{
				case Token::Type::begin_script_token:
					this->script_frame.Initialize(this->domain, &this->script);
					switch_to_state(State::script_frame_pending_state);
					break;

				case Token::Type::begin_array_token:
					{
						Array::Ref value = New<Array>();
						(*this->value) = value;

						this->array_frame = New<ArrayFrame>();
						this->array_frame->Initialize(this->domain, value);
						switch_to_state(State::array_frame_pending_state);
					}
					break;

				case Token::Type::begin_object_token:
					{
						Object::Ref value = New<Object>();
						(*this->value) = value;

						this->object_frame = New<ObjectFrame>();
						this->object_frame->Initialize(this->domain, value);
						switch_to_state(State::object_frame_pending_state);
					}
					break;

				case Token::Type::number_token:
					{
						Number::Ref value = New<Number>();
						(*this->value) = value;

						value->value = ((NumberToken*)token.item())->value;

						switch_to_state(State::done_state);
					}
					break;

				case Token::Type::string_token:
					{
						String::Ref value = New<String>();
						(*this->value) = value;

						value->value = ((StringToken*)token.item())->value;

						switch_to_state(State::done_state);
					}
					break;

				case Token::Type::bool_token:
					{
						Bool::Ref value = New<Bool>();
						(*this->value) = value;

						value->value = ((BoolToken*)token.item())->value;

						switch_to_state(State::done_state);
					}
					break;

				case Token::Type::null_token:
					{
						Null::Ref value = New<Null>();
						(*this->value) = value;

						switch_to_state(State::done_state);
					}
					break;

				default:
					ReadyForReadTokenPointerEvent::UndoReadNext(event);
					switch_to_state(State::start_state_error);
					break;
				}
			}
			break;

		case State::script_frame_pending_state:
			if (this->script_frame.Pending())
			{
				this->script_frame.Process(event, yield);
			}
			
			if (this->script_frame.Failed())
			{
				switch_to_state(State::script_frame_failed);
			}
			else if (this->script_frame.Succeeded())
			{
				UnicodeString::Ref element;

				bool success = this->script.Execute(this->domain, &element);
				if (success)
				{
					String::Ref value = New<String>();
					(*this->value) = value;

					value->value = element;
				}
				else
				{
					Null::Ref value = New<Null>();
					(*this->value) = value;
				}

				switch_to_state(State::done_state);
			}
			break;

		case State::array_frame_pending_state:
			if (this->array_frame->Pending())
			{
				this->array_frame->Process(event, yield);
			}

			if (this->array_frame->Failed())
			{
				switch_to_state(State::array_frame_failed);
			}
			else if (this->array_frame->Succeeded())
			{
				switch_to_state(State::done_state);
			}
			break;

		case State::object_frame_pending_state:
			if (this->object_frame->Pending())
			{
				this->object_frame->Process(event, yield);
			}

			if (this->object_frame->Failed())
			{
				switch_to_state(State::object_frame_failed);
			}
			else if (this->object_frame->Succeeded())
			{
				switch_to_state(State::done_state);
			}
			break;

		default:
			throw new Exception("Json::ValueFrame::Process unexpected state");
		}
	}
}