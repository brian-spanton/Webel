// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.Frame.h"
#include "Html.Types.h"
#include "Html.CharacterToken.h"
#include "Basic.INumberStream.h"
#include "Basic.MatchFrame.h"

namespace Html
{
	using namespace Basic;

	class Parser;

	class CharacterReferenceFrame : public Frame
	{
	private:
		enum State
		{
			unconsume_not_initialized_state = Start_State,
			start_state,
			number_started_state,
			receiving_number_state,
			number_stream_done_state,
			matching_name_state,
			cleanup_state,
			done_state = Succeeded_State,
		};

		Parser* parser;
		bool part_of_an_attribute;
		bool use_additional_allowed_character;
		Codepoint additional_allowed_character;
		UnicodeString* value;
		UnicodeString* unconsume;
		Codepoint number;
		Basic::Ref<INumberStream<Codepoint> > number_stream; // REF
		Html::StringMap::iterator match_value;
		Inline<MatchFrame<UnicodeString::Ref> > match_frame; // REF

	public:
		typedef Basic::Ref<CharacterReferenceFrame, IProcess> Ref;

		void Initialize(Parser* parser, bool part_of_an_attribute, bool use_additional_allowed_character, Codepoint additional_allowed_character, UnicodeString* value, UnicodeString* unconsume);

		virtual void IProcess::Process(IEvent* event, bool* yield);
	};
}
