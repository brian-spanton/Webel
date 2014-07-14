// Copyright © 2013 Brian Spanton

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

    CharacterReferenceFrame::CharacterReferenceFrame(Parser* parser) :
        number(0),
        match_frame(Html::globals->named_character_references_table, &this->match_value),
        number_stream(0),
        parser(parser)
    {
    }

    void CharacterReferenceFrame::reset(bool part_of_an_attribute, bool use_additional_allowed_character, Codepoint additional_allowed_character, UnicodeString* value, UnicodeStringRef not_consumed)
    {
        __super::reset();
        this->number = 0;
        this->match_frame.reset();
        this->number_stream = 0;

        this->part_of_an_attribute = part_of_an_attribute;
        this->use_additional_allowed_character = use_additional_allowed_character;
        this->additional_allowed_character = additional_allowed_character;
        this->value = value;
        this->not_consumed = not_consumed;
        this->not_consumed->reserve(0x100);
    }

    void CharacterReferenceFrame::write_element(Codepoint c)
    {
        this->not_consumed->write_element(c);
        WriteUnobserved(c);
    }

    void CharacterReferenceFrame::WriteUnobserved(Codepoint c)
    {
        switch (get_state())
        {
        case State::start_state:
            {
                if (this->use_additional_allowed_character && c == this->additional_allowed_character)
                {
                    switch_to_state(State::done_state);
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
                    switch_to_state(State::done_state);
                    break;

                case 0x0023:
                    switch_to_state(State::number_started_state);
                    break;

                default:
                    switch_to_state(State::matching_name_state);
                    WriteUnobserved(c);
                    return;
                }
            }
            break;

        case State::number_started_state:
            {
                switch(c)
                {
                case 0x0078:
                case 0x0058:
                    {
                        this->number_stream = std::make_shared<HexNumberStream<Codepoint, Codepoint> >(&this->number);
                        switch_to_state(State::receiving_number_state);
                    }
                    break;

                default:
                    {
                        this->number_stream = std::make_shared<DecNumberStream<Codepoint, Codepoint> >(&this->number);
                        switch_to_state(State::receiving_number_state);

                        WriteUnobserved(c);
                        return;
                    }
                    break;
                }
            }
            break;

        case State::receiving_number_state:
            {
                bool success = this->number_stream->WriteDigit(c);
                if (!success)
                {
                    if (this->number_stream->get_digit_count() > 0)
                    {
                        TranslationMap::iterator it = Html::globals->number_character_references_table.find(this->number);
                        if (it != Html::globals->number_character_references_table.end())
                        {
                            this->parser->ParseError("Html::CharacterReferenceFrame::handle_event character number reference not allowed");
                            this->number = it->second;
                        }
                        else if ((this->number >= 0xD800 && this->number <= 0xDFFF) || this->number > 0x10FFFF)
                        {
                            this->parser->ParseError("Html::CharacterReferenceFrame::handle_event character number reference out of range");
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
                            this->parser->ParseError("Html::CharacterReferenceFrame::handle_event character number reference out of range");
                        }

                        this->value->push_back(this->number);
                    }
                    else
                    {
                        this->parser->ParseError("Html::CharacterReferenceFrame::handle_event bad character number reference");
                    }

                    this->not_consumed->clear();

                    if (c != 0x003B)
                    {
                        this->parser->ParseError("character reference does not end with ;");
                        this->not_consumed->push_back(c);
                    }

                    switch_to_state(State::done_state);
                }
            }
            break;

        case State::matching_name_state:
            {
                this->match_frame.write_element(c);

                if (this->match_frame.in_progress())
                    break;

                if (this->match_value == Html::globals->named_character_references_table->end())
                {
                    // If no match can be made, then no characters are consumed, and nothing is returned. In this case,
                    // $ NYI:
                    // if the characters after the U+0026 AMPERSAND character (&) consist of a sequence of one or more
                    // alphanumeric ASCII characters followed by a U+003B SEMICOLON character (;), then this is a parse error.
                }
                else
                {
                    if (this->part_of_an_attribute && this->match_value->first->back() != 0x003B)
                    {
                        // If the character reference is being consumed as part of an attribute, and the last character matched
                        // is not a U+003B SEMICOLON character (;),
                        // $ NYI:
                        // and the next character is either a U+003D EQUALS SIGN character (=)
                        // or an alphanumeric ASCII character, then, for historical reasons, all the characters that were matched after
                        // the U+0026 AMPERSAND character (&) must be unconsumed, and nothing is returned. However, if this next character
                        // is in fact a U+003D EQUALS SIGN character (=), then this is a parse error, because some legacy user agents
                        // will misinterpret the markup in those cases.
                    }

                    this->value->append(this->match_value->second->begin(), this->match_value->second->end());

                    // we resolved a character reference, consume only the chars we used
                    this->not_consumed->erase(this->not_consumed->begin(), this->not_consumed->begin() + this->match_value->first->size());
                }

                switch_to_state(State::done_state);
            }
            break;

        default:
            throw FatalError("Html::CharacterReferenceFrame::handle_event unexpected state");
        }
    }

    void CharacterReferenceFrame::write_eof()
    {
        // this class requires EOF translated this way (see EOF use in write_element)
        write_element(EOF);
    }
}
