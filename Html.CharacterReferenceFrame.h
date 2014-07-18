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

    class CharacterReferenceFrame : public StateMachine, public UnitStream<Codepoint>
    {
    private:
        enum State
        {
            start_state = Start_State,
            number_started_state,
            receiving_number_state,
            matching_name_state,
            done_state = Succeeded_State,
        };

        Parser* parser;
        bool part_of_an_attribute;
        bool use_additional_allowed_character;
        Codepoint additional_allowed_character;
        UnicodeString* value;
        UnicodeStringRef leftovers;
        Codepoint number;
        std::shared_ptr<INumberStream<Codepoint> > number_stream;
        Html::StringMap::iterator match_value;
        MatchFrame<UnicodeStringRef> match_frame;

        void WriteUnobserved(Codepoint element);
        void Cleanup();

    public:
        CharacterReferenceFrame(Parser* parser);

        void reset(bool part_of_an_attribute, bool use_additional_allowed_character, Codepoint additional_allowed_character, UnicodeString* value, UnicodeStringRef leftovers);

        virtual void IStream<Codepoint>::write_element(Codepoint element);
        virtual void IStream<Codepoint>::write_eof();
    };
}
