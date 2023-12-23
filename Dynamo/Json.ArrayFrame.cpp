#include "stdafx.h"
#include "Json.ArrayFrame.h"
#include "Json.Globals.h"
#include "Json.Types.h"
#include "Json.Tokenizer.h"
#include "Json.Parser.h"
#include "Json.ValueFrame.h"
#include "Html.Globals.h"
#include "Html.ElementName.h"
#include "Html.ElementNode.h"

namespace Json
{
	using namespace Basic;

	void ArrayFrame::Initialize(Html::Node::Ref domain, Array* value)
	{
		__super::Initialize();

		this->domain = domain;
		this->element_domain = domain;
		this->value = value;
	}

	bool ArrayFrame::FindNextScriptElement()
	{
		Html::ElementNode::Ref script_element;

		bool success = this->script.Execute(this->domain, this->start_from, &script_element);
		if (!success)
		{
			this->start_from = 0;
			this->element_domain = this->domain;
			return false;
		}

		this->start_from = script_element;
		this->element_domain = script_element;
		return true;
	}

	void ArrayFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::expecting_first_element_state:
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

				case Token::Type::end_array_token:
					switch_to_state(State::done_state);
					break;

				default:
					ReadyForReadTokenPointerEvent::UndoReadNext(event);
					this->element_frame.Initialize(this->element_domain, &this->element);
					switch_to_state(State::element_frame_pending_state);
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
				if (event->get_type() != EventType::ready_for_read_token_pointer_event)
				{
					throw new Exception("unexpected event type");
				}

				bool success = FindNextScriptElement();
				if (success)
				{
					this->scripted_tokens = New<TokenVector>();

					ReadyForReadTokenPointerEvent* read_event = (ReadyForReadTokenPointerEvent*)event;
					this->element_source = read_event->element_source;
					this->element_source->AddObserver(this->scripted_tokens);
				}

				this->element_frame.Initialize(this->element_domain, &this->element);
				switch_to_state(State::script_execution_state);
			}
			break;

		case State::script_execution_state:
			if (this->element_frame.Pending())
			{
				this->element_frame.Process(event, yield);
			}

			if (this->element_frame.Failed())
			{
				switch_to_state(State::element_frame_failed);
			}
			else if (this->element_frame.Succeeded())
			{
				if (this->element_source.item() != 0)
				{
					this->value->elements.push_back(this->element);

					this->element_source->RemoveObserver(this->scripted_tokens);
					this->element_source = 0;

					while (true)
					{
						bool success = FindNextScriptElement();
						if (!success)
							break;

						this->element_frame.Initialize(this->element_domain, &this->element);

						FrameStream<Token::Ref>::Ref element_stream = New<FrameStream<Token::Ref> >();
						element_stream->Initialize(&this->element_frame);

						this->scripted_tokens->write_to(element_stream);

						if (this->element_frame.Failed())
						{
							switch_to_state(State::element_frame_failed);
						}
						else if (this->element_frame.Succeeded())
						{
							this->value->elements.push_back(this->element);
						}
						else
						{
							throw new Exception("unexpectedly still pending");
						}
					}
				}

				switch_to_state(State::expecting_value_separator_state);
			}
			break;

		case State::element_frame_pending_state:
			if (this->element_frame.Pending())
			{
				this->element_frame.Process(event, yield);
			}

			if (this->element_frame.Failed())
			{
				switch_to_state(State::element_frame_failed);
			}
			else if (this->element_frame.Succeeded())
			{
				this->value->elements.push_back(this->element);
				switch_to_state(State::expecting_value_separator_state);
			}
			break;

		case State::expecting_value_separator_state:
			{
				Token::Ref token;
				if (!ReadyForReadTokenPointerEvent::ReadNext(event, &token, yield))
					return;

				switch(token->type)
				{
				case Token::Type::end_array_token:
					switch_to_state(State::done_state);
					break;

				case Token::Type::value_separator_token:
					switch_to_state(State::expecting_next_element_state);
					break;

				default:
					ReadyForReadTokenPointerEvent::UndoReadNext(event);
					switch_to_state(State::expecting_value_separator_error);
					break;
				}
			}
			break;

		case State::expecting_next_element_state:
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

				default:
					ReadyForReadTokenPointerEvent::UndoReadNext(event);
					this->element_frame.Initialize(this->element_domain, &this->element);
					switch_to_state(State::element_frame_pending_state);
					break;
				}
			}
			break;

		default:
			throw new Exception("Json::ArrayFrame::Process unexpected state");
		}
	}
}