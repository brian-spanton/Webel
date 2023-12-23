#include "stdafx.h"
#include "Html.CharacterReferenceFrame.h"
#include "Basic.StreamFrame.h"
#include "Html.Globals.h"
#include "Html.CharacterToken.h"
#include "Basic.DecNumberStream.h"
#include "Basic.HexNumberStream.h"
#include "Html.Parser.h"

namespace Html
{
	using namespace Basic;

	void CharacterReferenceFrame::Initialize(Parser* parser, bool part_of_an_attribute, bool use_additional_allowed_character, Codepoint additional_allowed_character, UnicodeString* value, UnicodeString* unconsume)
	{
		__super::Initialize();
		this->parser = parser;
		this->part_of_an_attribute = part_of_an_attribute;
		this->use_additional_allowed_character = use_additional_allowed_character;
		this->additional_allowed_character = additional_allowed_character;
		this->value = value;
		this->number = 0;
		this->match_frame.Initialize(Html::globals->named_character_references_table, &this->match_value);
		this->unconsume = unconsume;
		this->number_stream = 0;
	}

	void CharacterReferenceFrame::Process(IEvent* event, bool* yield)
	{
		switch (frame_state())
		{
		case State::unconsume_not_initialized_state:
			{
				this->unconsume->reserve(0x100);

				Event::AddObserver<Codepoint>(event, this->unconsume);

				switch_to_state(State::start_state);
			}
			break;

		case State::start_state:
			{
				Codepoint c;
				if (!Event::ReadNext(event, &c, yield))
					return;

				if (this->use_additional_allowed_character && c == this->additional_allowed_character)
				{
					switch_to_state(State::cleanup_state);
				}
				else switch (c)
				{
				case 0x0009:
				case 0x000A:
				case 0x000C:
				case 0x0020:
				case 0x003C:
				case 0x0026:
				case EOF:
					switch_to_state(State::cleanup_state);
					break;

				case 0x0023:
					switch_to_state(State::number_started_state);
					break;

				default:
					Event::UndoReadNext(event);
					switch_to_state(State::matching_name_state);
					break;
				}
			}
			break;

		case State::number_started_state:
			{
				Codepoint c;
				if (!Event::ReadNext(event, &c, yield))
					return;

				switch(c)
				{
				case 0x0078:
				case 0x0058:
					{
						HexNumberStream<Codepoint, Codepoint>::Ref stream = New<HexNumberStream<Codepoint, Codepoint> >();
						stream->Initialize(&this->number);

						this->number_stream = stream;
						switch_to_state(State::receiving_number_state);
					}
					break;

				default:
					{
						Event::UndoReadNext(event);

						DecNumberStream<Codepoint, Codepoint>::Ref stream = New<DecNumberStream<Codepoint, Codepoint> >();
						stream->Initialize(&this->number);

						this->number_stream = stream;
						switch_to_state(State::receiving_number_state);
					}
					break;
				}
			}
			break;

		case State::receiving_number_state:
			{
				Codepoint c;
				if (!Event::ReadNext(event, &c, yield))
					return;

				bool success = this->number_stream->WriteDigit(c);
				if (!success)
				{
					if (c != 0x003B)
					{
						Event::UndoReadNext(event);
						this->parser->ParseError("character reference does not end with ;");
					}

					switch_to_state(State::number_stream_done_state);
				}
			}
			break;

		case State::number_stream_done_state:
			{
				if (this->number_stream->get_digit_count() > 0)
				{
					TranslationMap::iterator it = Html::globals->number_character_references_table->find(this->number);
					if (it != Html::globals->number_character_references_table->end())
					{
						this->parser->ParseError("Html::CharacterReferenceFrame::Process character number reference not allowed");
						this->number = it->second;
					}
					else if ((this->number >= 0xD800 && this->number <= 0xDFFF) || this->number > 0x10FFFF)
					{
						this->parser->ParseError("Html::CharacterReferenceFrame::Process character number reference out of range");
						this->number = 0xFFFD;
					}
					else if ((this->number >= 0x0001 && this->number <= 0x0008) ||
						(this->number >= 0x000E && this->number <= 0x001F) ||
						(this->number >= 0x007F && this->number <= 0x009F) ||
						(this->number >= 0xFDD0 && this->number <= 0xFDEF) ||
						this->number == 0x000B ||
						this->number == 0xFFFE ||
						this->number == 0xFFFF ||
						this->number == 0x1FFFE ||
						this->number == 0x1FFFF ||
						this->number == 0x2FFFE ||
						this->number == 0x2FFFF ||
						this->number == 0x3FFFE ||
						this->number == 0x3FFFF ||
						this->number == 0x4FFFE ||
						this->number == 0x4FFFF ||
						this->number == 0x5FFFE ||
						this->number == 0x5FFFF ||
						this->number == 0x6FFFE ||
						this->number == 0x6FFFF ||
						this->number == 0x7FFFE ||
						this->number == 0x7FFFF ||
						this->number == 0x8FFFE ||
						this->number == 0x8FFFF ||
						this->number == 0x9FFFE ||
						this->number == 0x9FFFF ||
						this->number == 0xAFFFE ||
						this->number == 0xAFFFF ||
						this->number == 0xBFFFE ||
						this->number == 0xBFFFF ||
						this->number == 0xCFFFE ||
						this->number == 0xCFFFF ||
						this->number == 0xDFFFE ||
						this->number == 0xDFFFF ||
						this->number == 0xEFFFE ||
						this->number == 0xEFFFF ||
						this->number == 0xFFFFE ||
						this->number == 0xFFFFF ||
						this->number == 0x10FFFE ||
						this->number == 0x10FFFF)
					{
						this->parser->ParseError("Html::CharacterReferenceFrame::Process character number reference out of range");
					}

					this->value->push_back(this->number);
				}
				else
				{
					this->parser->ParseError("Html::CharacterReferenceFrame::Process bad character number reference");
				}

				switch_to_state(State::cleanup_state);
			}
			break;

		case State::matching_name_state:
			if (this->match_frame.Pending())
			{
				this->match_frame.Process(event, yield);
			}
			else if (this->match_frame.Failed())
			{
				throw new Exception("match_frame.Failed()");
			}
			else
			{
				if (this->match_value == Html::globals->named_character_references_table->end())
				{
					// If no match can be made, then no characters are consumed, and nothing is returned. In this case,
					// $ nyi:
					// if the characters after the U+0026 AMPERSAND character (&) consist of a sequence of one or more
					// alphanumeric ASCII characters followed by a U+003B SEMICOLON character (;), then this is a parse error.
				}
				else
				{
					if (this->part_of_an_attribute && this->match_value->first->back() != 0x003B)
					{
						// If the character reference is being consumed as part of an attribute, and the last character matched
						// is not a U+003B SEMICOLON character (;),
						// $ nyi :
						// and the next character is either a U+003D EQUALS SIGN character (=)
						// or an alphanumeric ASCII character, then, for historical reasons, all the characters that were matched after
						// the U+0026 AMPERSAND character (&) must be unconsumed, and nothing is returned. However, if this next character
						// is in fact a U+003D EQUALS SIGN character (=), then this is a parse error, because some legacy user agents
						// will misinterpret the markup in those cases.
					}

					this->value->append(this->match_value->second->begin(), this->match_value->second->end());
				}

				switch_to_state(State::cleanup_state);
			}
			break;

		case State::cleanup_state:
			{
				Event::RemoveObserver<Codepoint>(event, this->unconsume);

				if (this->unconsume->size() > 0)
				{
					if (this->value->size() == 0)
					{
						// we didn't resolve to any known character reference, so allow everything we saw to be unconsumed
					}
					else if (this->match_value != Html::globals->named_character_references_table->end())
					{
						// we resolved a character reference, but may have seen more chars than we ended up using
						this->unconsume->erase(this->unconsume->begin(), this->unconsume->begin() + this->match_value->first->size());
					}
					else
					{
						// numeric reference or something, nothing to unconsume
						this->unconsume->clear();
					}
				}

				switch_to_state(State::done_state);
			}
			break;

		default:
			throw new Exception("Html::CharacterReferenceFrame::Process unexpected state");
		}
	}
}
