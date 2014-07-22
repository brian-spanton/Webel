// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.Tokenizer.h"
#include "Html.Globals.h"
#include "Html.Types.h"
#include "Html.StartTagToken.h"
#include "Html.EndTagToken.h"
#include "Html.CharacterToken.h"
#include "Html.EndOfFileToken.h"
#include "Html.Parser.h"
#include "Basic.Globals.h"

namespace Html
{
    using namespace Basic;

    Tokenizer::Tokenizer(Parser* parser, std::shared_ptr<IStream<TokenRef> > output) :
        temporary_buffer(std::make_shared<UnicodeString>()),
        markup_declaration_open(std::make_shared<UnicodeString>()),
        after_doctype_name(std::make_shared<UnicodeString>()),
        parser(parser),
        output(output),
        char_ref_frame(this->parser) // initialization is in order of declaration in class def
    {
    }

    void Tokenizer::SwitchToState(TokenizerState state)
    {
        // first check state we are leaving for special cases...

        if (this->get_state() == attribute_name_state)
            InsertAttribute();

        // then check state we are entering for special cases...

        switch (state)
        {
        case character_reference_in_attribute_value_state:
            this->attribute_value_state = (TokenizerState)this->get_state();
            this->character_reference = std::make_shared<UnicodeString>();
            this->character_reference_leftovers = std::make_shared<UnicodeString>();
            this->char_ref_frame.reset(true, this->use_additional_allowed_character, this->additional_allowed_character, this->character_reference.get(), this->character_reference_leftovers);
            break;
        }

        __super::switch_to_state(state);
    }

    void Tokenizer::consume_leftovers()
    {
        // store in local variable since we are about to recurse and change character_reference_leftovers
        UnicodeStringRef leftovers = this->character_reference_leftovers;

        for (UnicodeString::iterator it = leftovers->begin(); it != leftovers->end(); it++)
            this->write_element(*it);
    }

    void Tokenizer::InsertAttribute()
    {
        if (this->current_attribute_name.get() == 0 || this->current_attribute_name->size() == 0)
        {
            Basic::globals->HandleError("Tokenizer::InsertAttribute 1", 0);
            return;
        }

        if (this->current_tag_token.get() == 0)
        {
            Basic::globals->HandleError("Tokenizer::InsertAttribute 2", 0);
            return;
        }

        StringMap::value_type value(this->current_attribute_name, std::make_shared<UnicodeString>());

        std::pair<StringMap::iterator, bool> result = this->current_tag_token->attributes.insert(value);
        if (!result.second)
            ParseError("duplicate attribute");

        this->current_attribute = result.first;
        this->current_attribute_name = 0;
    }

    bool Tokenizer::IsAppropriate(Token* token)
    {
        if (token->type != Token::Type::end_tag_token)
            return false;

        EndTagToken* end_tag = static_cast<EndTagToken*>(token);

        if (this->last_start_tag_name != end_tag->name)
            return false;

        return true;
    }

    void Tokenizer::EmitCharacter(Codepoint c)
    {
        std::shared_ptr<CharacterToken> token = std::make_shared<CharacterToken>();
        token->data = c;

        Emit(token);
    }

    void Tokenizer::EmitCurrentTagToken()
    {
        if (this->current_tag_token.get() == 0)
            throw FatalError("Tokenizer::EmitCurrentTagToken should have valid this->current_tag_token");

        if (this->get_state() == attribute_name_state)
            InsertAttribute();

        Emit(this->current_tag_token);

        this->current_tag_token = 0;
    }

    void Tokenizer::Emit(TokenRef token)
    {
        StartTagToken* start_tag_to_acknowledge = 0;

        switch (token->type)
        {
        case Token::Type::start_tag_token:
            {
                StartTagToken* start_tag = static_cast<StartTagToken*>(token.get());
                if (start_tag->self_closing)
                    start_tag_to_acknowledge = start_tag;

                this->last_start_tag_name = start_tag->name;
            }
            break;

        case Token::Type::end_tag_token:
            {
                EndTagToken* end_tag = static_cast<EndTagToken*>(token.get());

                if (end_tag->attributes.size() > 0)
                    ParseError("end tag token with attributes");

                if (end_tag->self_closing == true)
                    ParseError("self closing end tag");
            }
            break;
        }

        this->output->write_element(token);

        // When a start tag token is emitted with its self-closing flag set, if the flag is not acknowledged
        // when it is processed by the tree construction stage, that is a parse error.
        if (start_tag_to_acknowledge != 0 && start_tag_to_acknowledge->acknowledged == false)
        {
            ParseError("unacknowledged self-closing start tag");
        }
    }

    void Tokenizer::write_eof()
    {
        // this class requires EOF translated this way (see EOF use in write_element)
        write_element(EOF);
    }

    void Tokenizer::write_element(Codepoint c)
    {
        switch (this->get_state())
        {
        case data_state:
            {
                switch(c)
                {
                case 0x0026:
                    this->character_reference = std::make_shared<UnicodeString>();
                    this->character_reference_leftovers = std::make_shared<UnicodeString>();
                    this->char_ref_frame.reset(false, false, 0, this->character_reference.get(), this->character_reference_leftovers);
                    SwitchToState(character_reference_in_data_state);
                    break;

                case 0X003C:
                    SwitchToState(tag_open_state);
                    break;

                case 0x0000:
                    ParseError("data _stateNULL");
                    EmitCharacter(c);
                    break;

                case EOF:
                    Emit(Html::globals->eof_token);
                    break;

                default:
                    EmitCharacter(c);
                    break;
                }
            }
            break;

        case character_reference_in_data_state:
            {
                this->char_ref_frame.write_element(c);

                if (this->char_ref_frame.in_progress())
                    break;

                if (this->character_reference->size() == 0)
                {
                    EmitCharacter(0x0026);
                }
                else
                {
                    for (UnicodeString::iterator it = this->character_reference->begin(); it != this->character_reference->end(); it++)
                        EmitCharacter(*it);
                }

                SwitchToState(data_state);

                consume_leftovers();
                return;
            }
            break;

        case RCDATA_state:
            {
                switch (c)
                {
                case 0x0026:
                    this->character_reference = std::make_shared<UnicodeString>();
                    this->character_reference_leftovers = std::make_shared<UnicodeString>();
                    this->char_ref_frame.reset(false, false, 0, this->character_reference.get(), this->character_reference_leftovers);
                    SwitchToState(character_reference_in_RCDATA_state);
                    break;

                case 0x003C:
                    SwitchToState(RCDATA_less_than_sign_state);
                    break;

                case 0x0000:
                    ParseError("RCDATA _stateNULL");
                    EmitCharacter(0xFFFD);
                    break;

                case EOF:
                    Emit(Html::globals->eof_token);
                    break;

                default:
                    EmitCharacter(c);
                    break;
                }
            }
            break;

        case character_reference_in_RCDATA_state:
            {
                this->char_ref_frame.write_element(c);

                if (this->char_ref_frame.in_progress())
                    break;

                if (this->character_reference->size() == 0)
                {
                    EmitCharacter(0x0026);
                }
                else
                {
                    for (UnicodeString::iterator it = this->character_reference->begin(); it != this->character_reference->end(); it++)
                        EmitCharacter(*it);
                }

                SwitchToState(RCDATA_state);

                consume_leftovers();
                return;
            }
            break;

        case RAWTEXT_state:
            {
                switch (c)
                {
                case 0x003C:
                    SwitchToState(RAWTEXT_less_than_sign_state);
                    break;

                case 0x0000:
                    ParseError(c);
                    EmitCharacter(0xFFFD);
                    break;

                case EOF:
                    Emit(Html::globals->eof_token);
                    break;

                default:
                    EmitCharacter(c);
                    break;
                }
            }
            break;

        case script_data_state:
            {
                switch (c)
                {
                case 0x003C:
                    SwitchToState(script_data_less_than_sign_state);
                    break;

                case 0x0000:
                    ParseError(c);
                    EmitCharacter(0xFFFD);
                    break;

                case EOF:
                    Emit(Html::globals->eof_token);
                    break;

                default:
                    EmitCharacter(c);
                    break;
                }
            }
            break;

        case PLAINTEXT_state:
            {
                switch (c)
                {
                case 0x0000:
                    ParseError(c);
                    EmitCharacter(0xFFFD);
                    break;

                case EOF:
                    Emit(Html::globals->eof_token);
                    break;

                default:
                    EmitCharacter(c);
                    break;
                }
            }
            break;

        case tag_open_state:
            {
                if (c >= 'A' && c <= 'Z')
                {
                    this->current_tag_token = std::make_shared<StartTagToken>();
                    this->current_tag_token->name->push_back(c + 0x0020);
                    SwitchToState(tag_name_state);
                }
                else if (c >= 'a' && c <= 'z')
                {
                    this->current_tag_token = std::make_shared<StartTagToken>();
                    this->current_tag_token->name->push_back(c);
                    SwitchToState(tag_name_state);
                }
                else switch (c)
                {
                case 0x0021:
                    this->markup_declaration_open->clear();
                    this->markup_declaration_open->reserve(7);
                    SwitchToState(markup_declaration_open_state);
                    break;

                case 0x002F:
                    SwitchToState(end_tag_open_state);
                    break;

                case 0x003F:
                    ParseError(c);
                    SwitchToState(bogus_comment_state);

                    this->comment_token = std::make_shared<CommentToken>();
                    this->comment_token->data->push_back(0x003F);
                    break;

                default:
                    ParseError(c);
                    SwitchToState(data_state);

                    EmitCharacter(0x003C);

                    write_element(c);
                    return;
                }
            }
            break;

        case end_tag_open_state:
            {
                if (c >= 'A' && c <= 'Z')
                {
                    this->current_tag_token = std::make_shared<EndTagToken>();
                    this->current_tag_token->name->push_back(c + 0x0020);
                    SwitchToState(tag_name_state);
                }
                else if (c >= 'a' && c <= 'z')
                {
                    this->current_tag_token = std::make_shared<EndTagToken>();
                    this->current_tag_token->name->push_back(c);
                    SwitchToState(tag_name_state);
                }
                else switch (c)
                {
                case 0x003E:
                    ParseError(c);
                    SwitchToState(data_state);
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);

                    EmitCharacter(0x003C);
                    EmitCharacter(0x002F);

                    write_element(c);
                    return;

                default:
                    ParseError(c);
                    SwitchToState(bogus_comment_state);

                    this->comment_token = std::make_shared<CommentToken>();
                    this->comment_token->data->push_back(c);
                    break;
                }
            }
            break;

        case tag_name_state:
            {
                if (c >= 'A' && c <= 'Z')
                {
                    this->current_tag_token->name->push_back(c + 0x0020);
                }
                else switch (c)
                {
                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x0020:
                    SwitchToState(before_attribute_name_state);
                    break;

                case 0x002F:
                    SwitchToState(self_closing_start_tag_state);
                    break;

                case 0x003E:
                    SwitchToState(data_state);
                    EmitCurrentTagToken();
                    break;

                case 0x0000:
                    ParseError(c);
                    this->current_tag_token->name->push_back(0xFFFD);
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    write_element(c);
                    return;

                default:
                    this->current_tag_token->name->push_back(c);
                    break;
                }
            }
            break;

        case RCDATA_less_than_sign_state:
            {
                switch (c)
                {
                case 0x002F:
                    this->temporary_buffer->clear();
                    SwitchToState(RCDATA_end_tag_open_state);
                    break;

                default:
                    SwitchToState(RCDATA_state);
                    EmitCharacter(0x003C);
                    write_element(c);
                    return;
                }
            }
            break;

        case RCDATA_end_tag_open_state:
            {
                if (c >= 'A' && c <= 'Z')
                {
                    this->current_tag_token = std::make_shared<EndTagToken>();
                    this->current_tag_token->name->push_back(c + 0x0020);
                    this->temporary_buffer->push_back(c);
                    SwitchToState(RCDATA_end_tag_name_state);
                }
                else if (c >= 'a' && c <= 'z')
                {
                    this->current_tag_token = std::make_shared<EndTagToken>();
                    this->current_tag_token->name->push_back(c);
                    this->temporary_buffer->push_back(c);
                    SwitchToState(RCDATA_end_tag_name_state);
                }
                else
                {
                    SwitchToState(RCDATA_state);
                    EmitCharacter(0x003C);
                    EmitCharacter(0x002F);
                    write_element(c);
                    return;
                }
            }
            break;

        case RCDATA_end_tag_name_state:
            {
                bool anything_else = false;

                if (c >= 'A' && c <= 'Z')
                {
                    this->current_tag_token->name->push_back(c + 0x0020);
                    this->temporary_buffer->push_back(c);
                }
                else if (c >= 'a' && c <= 'z')
                {
                    this->current_tag_token->name->push_back(c);
                    this->temporary_buffer->push_back(c);
                }
                else if (IsAppropriate(this->current_tag_token.get()))
                {
                    switch (c)
                    {
                    case 0x0009:
                    case 0x000A:
                    case 0x000C:
                    case 0x0020:
                        SwitchToState(before_attribute_name_state);
                        break;

                    case 0x002F:
                        SwitchToState(self_closing_start_tag_state);
                        break;

                    case 0x003E:
                        SwitchToState(data_state);
                        EmitCurrentTagToken();
                        break;

                    default:
                        anything_else = true;
                        break;
                    }
                }
                else
                {
                    anything_else = true;
                }

                if (anything_else)
                {
                    SwitchToState(RCDATA_state);
                    EmitCharacter(0x003C);
                    EmitCharacter(0x002F);

                    for (UnicodeString::iterator it = this->temporary_buffer->begin(); it != this->temporary_buffer->end(); it++)
                    {
                        EmitCharacter(*it);
                    }

                    write_element(c);
                    return;
                }
            }
            break;

        case RAWTEXT_less_than_sign_state:
            {
                switch (c)
                {
                case 0x002F:
                    this->temporary_buffer->clear();
                    SwitchToState(RAWTEXT_end_tag_open_state);
                    break;

                default:
                    SwitchToState(RAWTEXT_state);
                    EmitCharacter(0x003C);
                    write_element(c);
                    return;
                }
            }
            break;

        case RAWTEXT_end_tag_open_state:
            {
                if (c >= 'A' && c <= 'Z')
                {
                    this->current_tag_token = std::make_shared<EndTagToken>();
                    this->current_tag_token->name->push_back(c + 0x0020);
                    this->temporary_buffer->push_back(c);
                    SwitchToState(RAWTEXT_end_tag_name_state);
                }
                else if (c >= 'a' && c <= 'z')
                {
                    this->current_tag_token = std::make_shared<EndTagToken>();
                    this->current_tag_token->name->push_back(c);
                    this->temporary_buffer->push_back(c);
                    SwitchToState(RAWTEXT_end_tag_name_state);
                }
                else
                {
                    SwitchToState(RAWTEXT_state);
                    EmitCharacter(0x003C);
                    EmitCharacter(0x002F);
                    write_element(c);
                    return;
                }
            }
            break;

        case RAWTEXT_end_tag_name_state:
            {
                bool anything_else = false;

                if (c >= 'A' && c <= 'Z')
                {
                    this->current_tag_token->name->push_back(c + 0x0020);
                    this->temporary_buffer->push_back(c);
                }
                else if (c >= 'a' && c <= 'z')
                {
                    this->current_tag_token->name->push_back(c);
                    this->temporary_buffer->push_back(c);
                }
                else if (IsAppropriate(this->current_tag_token.get()))
                {
                    switch (c)
                    {
                    case 0x0009:
                    case 0x000A:
                    case 0x000C:
                    case 0x0020:
                        SwitchToState(before_attribute_name_state);
                        break;

                    case 0x002F:
                        SwitchToState(self_closing_start_tag_state);
                        break;

                    case 0x003E:
                        SwitchToState(data_state);
                        EmitCurrentTagToken();
                        break;

                    default:
                        anything_else = true;
                        break;
                    }
                }
                else
                {
                    anything_else = true;
                }

                if (anything_else)
                {
                    SwitchToState(RAWTEXT_state);
                    EmitCharacter(0x003C);
                    EmitCharacter(0x002F);

                    for (UnicodeString::iterator it = this->temporary_buffer->begin(); it != this->temporary_buffer->end(); it++)
                    {
                        EmitCharacter(*it);
                    }

                    write_element(c);
                    return;
                }
            }
            break;

        case script_data_less_than_sign_state:
            {
                switch (c)
                {
                case 0x002F:
                    this->temporary_buffer->clear();
                    SwitchToState(script_data_end_tag_open_state);
                    break;

                case 0x0021:
                    SwitchToState(script_data_escape_start_state);
                    EmitCharacter(0x003C);
                    EmitCharacter(0x0021);
                    break;

                default:
                    SwitchToState(script_data_state);
                    EmitCharacter(0x003C);
                    write_element(c);
                    return;
                }
            }
            break;

        case script_data_end_tag_open_state:
            {
                if (c >= 'A' && c <= 'Z')
                {
                    this->current_tag_token = std::make_shared<EndTagToken>();
                    this->current_tag_token->name->push_back(c + 0x0020);
                    this->temporary_buffer->push_back(c);
                    SwitchToState(script_data_end_tag_name_state);
                }
                else if (c >= 'a' && c <= 'z')
                {
                    this->current_tag_token = std::make_shared<EndTagToken>();
                    this->current_tag_token->name->push_back(c);
                    this->temporary_buffer->push_back(c);
                    SwitchToState(script_data_end_tag_name_state);
                }
                else
                {
                    SwitchToState(script_data_state);
                    EmitCharacter(0x003C);
                    EmitCharacter(0x002F);
                    write_element(c);
                    return;
                }
            }
            break;

        case script_data_end_tag_name_state:
            {
                bool anything_else = false;

                if (c >= 'A' && c <= 'Z')
                {
                    this->current_tag_token->name->push_back(c + 0x0020);
                    this->temporary_buffer->push_back(c);
                }
                else if (c >= 'a' && c <= 'z')
                {
                    this->current_tag_token->name->push_back(c);
                    this->temporary_buffer->push_back(c);
                }
                else if (IsAppropriate(this->current_tag_token.get()))
                {
                    switch (c)
                    {
                    case 0x0009:
                    case 0x000A:
                    case 0x000C:
                    case 0x0020:
                        SwitchToState(before_attribute_name_state);
                        break;

                    case 0x002F:
                        SwitchToState(self_closing_start_tag_state);
                        break;

                    case 0x003E:
                        SwitchToState(data_state);
                        EmitCurrentTagToken();
                        break;

                    default:
                        anything_else = true;
                        break;
                    }
                }
                else
                {
                    anything_else = true;
                }

                if (anything_else)
                {
                    SwitchToState(script_data_state);
                    EmitCharacter(0x003C);
                    EmitCharacter(0x002F);

                    for (UnicodeString::iterator it = this->temporary_buffer->begin(); it != this->temporary_buffer->end(); it++)
                    {
                        EmitCharacter(*it);
                    }

                    write_element(c);
                    return;
                }
            }
            break;

        case script_data_escape_start_state:
            {
                switch (c)
                {
                case 0x002D:
                    SwitchToState(script_data_escape_start_dash_state);
                    EmitCharacter(0x002D);
                    break;

                default:
                    SwitchToState(script_data_state);
                    write_element(c);
                    return;
                }
            }
            break;

        case script_data_escape_start_dash_state:
            {
                switch (c)
                {
                case 0x002D:
                    SwitchToState(script_data_escaped_dash_dash_state);
                    EmitCharacter(0x002D);
                    break;

                default:
                    SwitchToState(script_data_state);
                    write_element(c);
                    return;
                }
            }
            break;

        case script_data_escaped_state:
            {
                switch (c)
                {
                case 0x002D:
                    SwitchToState(script_data_escaped_dash_state);
                    EmitCharacter(0x002D);
                    break;

                case 0x003C:
                    SwitchToState(script_data_escaped_less_than_sign_state);
                    break;

                case 0x0000:
                    ParseError(c);
                    EmitCharacter(0xFFFD);
                    break;

                case EOF:
                    SwitchToState(data_state);
                    ParseError(c);
                    write_element(c);
                    return;

                default:
                    EmitCharacter(c);
                    break;
                }
            }
            break;

        case script_data_escaped_dash_state:
            {
                switch (c)
                {
                case 0x002D:
                    SwitchToState(script_data_escaped_dash_dash_state);
                    EmitCharacter(0x002D);
                    break;

                case 0x003C:
                    SwitchToState(script_data_escaped_less_than_sign_state);
                    break;

                case 0x0000:
                    ParseError(c);
                    SwitchToState(script_data_escaped_state);
                    EmitCharacter(0xFFFD);
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    write_element(c);
                    return;

                default:
                    SwitchToState(script_data_escaped_state);
                    EmitCharacter(c);
                    break;
                }
            }
            break;

        case script_data_escaped_dash_dash_state:
            {
                switch (c)
                {
                case 0x002D:
                    EmitCharacter(0x002D);
                    break;

                case 0x003C:
                    SwitchToState(script_data_escaped_less_than_sign_state);
                    break;

                case 0x003E:
                    SwitchToState(script_data_state);
                    EmitCharacter(0x003E);
                    break;

                case 0x0000:
                    ParseError(c);
                    SwitchToState(script_data_escaped_state);
                    EmitCharacter(0xFFFD);
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    write_element(c);
                    return;

                default:
                    SwitchToState(script_data_escaped_state);
                    EmitCharacter(c);
                    break;
                }
            }
            break;

        case script_data_escaped_less_than_sign_state:
            {
                if (c >= 'A' && c <= 'Z')
                {
                    this->temporary_buffer->clear();
                    this->temporary_buffer->push_back(c + 0x0020);

                    SwitchToState(script_data_double_escape_start_state);
                    EmitCharacter(0x003C);
                    EmitCharacter(c);
                }
                else if (c >= 'a' && c <= 'z')
                {
                    this->temporary_buffer->clear();
                    this->temporary_buffer->push_back(c);

                    SwitchToState(script_data_double_escape_start_state);
                    EmitCharacter(0x003C);
                    EmitCharacter(c);
                }
                else switch (c)
                {
                case 0x002D:
                    this->temporary_buffer->clear();
                    SwitchToState(script_data_escaped_end_tag_open_state);
                    break;

                default:
                    SwitchToState(script_data_escaped_state);
                    EmitCharacter(0x003C);
                    write_element(c);
                    return;
                }
            }
            break;

        case script_data_escaped_end_tag_open_state:
            {
                if (c >= 'A' && c <= 'Z')
                {
                    this->current_tag_token = std::make_shared<EndTagToken>();
                    this->current_tag_token->name->push_back(c + 0x0020);

                    this->temporary_buffer->push_back(c);

                    SwitchToState(script_data_escaped_end_tag_name_state);
                }
                else if (c >= 'a' && c <= 'z')
                {
                    this->current_tag_token = std::make_shared<EndTagToken>();
                    this->current_tag_token->name->push_back(c);

                    this->temporary_buffer->push_back(c);

                    SwitchToState(script_data_escaped_end_tag_name_state);
                }
                else
                {
                    SwitchToState(script_data_escaped_state);
                    EmitCharacter(0x003C);
                    EmitCharacter(0x002F);
                    write_element(c);
                    return;
                }
            }
            break;

        case script_data_escaped_end_tag_name_state:
            {
                bool anything_else = false;

                if (c >= 'A' && c <= 'Z')
                {
                    this->current_tag_token->name->push_back(c + 0x0020);
                    this->temporary_buffer->push_back(c);
                }
                else if (c >= 'a' && c <= 'z')
                {
                    this->current_tag_token->name->push_back(c);
                    this->temporary_buffer->push_back(c);
                }
                else if (IsAppropriate(this->current_tag_token.get()))
                {
                    switch (c)
                    {
                    case 0x0009:
                    case 0x000A:
                    case 0x000C:
                    case 0x0020:
                        SwitchToState(before_attribute_name_state);
                        break;

                    case 0x002F:
                        SwitchToState(self_closing_start_tag_state);
                        break;

                    case 0x003E:
                        SwitchToState(data_state);
                        EmitCurrentTagToken();
                        break;

                    default:
                        anything_else = true;
                        break;
                    }
                }
                else
                {
                    anything_else = true;
                }

                if (anything_else)
                {
                    SwitchToState(script_data_escaped_state);
                    EmitCharacter(0x003C);
                    EmitCharacter(0x002F);

                    for (UnicodeString::iterator it = this->temporary_buffer->begin(); it != this->temporary_buffer->end(); it++)
                    {
                        EmitCharacter(*it);
                    }

                    write_element(c);
                    return;
                }
            }
            break;

        case script_data_double_escape_start_state:
            {
                if (c >= 'A' && c <= 'Z')
                {
                    this->temporary_buffer->push_back(c + 0x0020);
                    EmitCharacter(c);
                }
                else if (c >= 'a' && c <= 'z')
                {
                    this->temporary_buffer->push_back(c);
                    EmitCharacter(c);
                }
                else switch (c)
                {
                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x0020:
                case 0x002F:
                case 0x003E:
                    {
                        if (equals<UnicodeString, false>(this->temporary_buffer.get(), Html::globals->Script.get()))
                            SwitchToState(script_data_double_escaped_state);
                        else
                            SwitchToState(script_data_escaped_state);

                        EmitCharacter(c);
                    }
                    break;

                default:
                    SwitchToState(script_data_escaped_state);
                    write_element(c);
                    return;
                }
            }
            break;

        case script_data_double_escaped_state:
            {
                switch (c)
                {
                case 0x002D:
                    SwitchToState(script_data_double_escaped_dash_state);
                    EmitCharacter(0x002D);
                    break;

                case 0x003C:
                    SwitchToState(script_data_double_escaped_less_than_sign_state);
                    EmitCharacter(0x003C);
                    break;

                case 0x0000:
                    ParseError(c);
                    EmitCharacter(0xFFFD);
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    write_element(c);
                    return;

                default:
                    EmitCharacter(c);
                    break;
                }
            }
            break;

        case script_data_double_escaped_dash_state:
            {
                switch (c)
                {
                case 0x002D:
                    SwitchToState(script_data_double_escaped_dash_dash_state);
                    EmitCharacter(0x002D);
                    break;

                case 0x003C:
                    SwitchToState(script_data_double_escaped_less_than_sign_state);
                    EmitCharacter(0x003C);
                    break;

                case 0x0000:
                    ParseError(c);
                    SwitchToState(script_data_double_escaped_state);
                    EmitCharacter(0xFFFD);
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    write_element(c);
                    return;

                default:
                    SwitchToState(script_data_double_escaped_state);
                    break;
                }
            }
            break;

        case script_data_double_escaped_dash_dash_state:
            {
                switch (c)
                {
                case 0x002D:
                    EmitCharacter(0x002D);
                    break;

                case 0x003C:
                    SwitchToState(script_data_double_escaped_less_than_sign_state);
                    EmitCharacter(0x003C);
                    break;

                case 0x003E:
                    SwitchToState(script_data_state);
                    EmitCharacter(0x003E);
                    break;

                case 0x0000:
                    ParseError(c);
                    SwitchToState(script_data_double_escaped_state);
                    EmitCharacter(0xFFFD);
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    write_element(c);
                    return;

                default:
                    SwitchToState(script_data_double_escaped_state);
                    EmitCharacter(c);
                    break;
                }
            }
            break;

        case script_data_double_escaped_less_than_sign_state:
            {
                switch (c)
                {
                case 0x002F:
                    this->temporary_buffer->clear();
                    SwitchToState(script_data_double_escape_end_state);
                    EmitCharacter(0x002F);
                    break;

                default:
                    SwitchToState(script_data_double_escaped_state);
                    write_element(c);
                    return;
                }
            }
            break;

        case script_data_double_escape_end_state:
            {
                if (c >= 'A' && c <= 'Z')
                {
                    this->temporary_buffer->push_back(c + 0x0020);
                    EmitCharacter(c);
                }
                else if (c >= 'a' && c <= 'z')
                {
                    this->temporary_buffer->push_back(c);
                    EmitCharacter(c);
                }
                else switch (c)
                {
                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x0020:
                case 0x002F:
                case 0x003E:
                    {
                        if (equals<UnicodeString, false>(this->temporary_buffer.get(), Html::globals->Script.get()))
                            SwitchToState(script_data_escaped_state);
                        else
                            SwitchToState(script_data_double_escaped_state);

                        EmitCharacter(c);
                    }
                    break;

                default:
                    SwitchToState(script_data_double_escaped_state);
                    write_element(c);
                    return;
                }
            }
            break;

        case before_attribute_name_state:
            {
                bool anything_else = false;

                if (c >= 'A' && c <= 'Z')
                {
                    this->current_attribute_name = std::make_shared<UnicodeString>();
                    this->current_attribute_name->push_back(c + 0x0020);
                    SwitchToState(attribute_name_state);
                }
                else switch (c)
                {
                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x0020:
                    break;

                case 0x002F:
                    SwitchToState(self_closing_start_tag_state);
                    break;

                case 0x003E:
                    SwitchToState(data_state);
                    EmitCurrentTagToken();
                    break;

                case 0x0000:
                    ParseError(c);
                    this->current_attribute_name = std::make_shared<UnicodeString>();
                    this->current_attribute_name->push_back(0xFFFD);
                    SwitchToState(attribute_name_state);
                    break;

                case 0x0022:
                case 0x0027:
                case 0x003C:
                case 0x003D:
                    ParseError(c);
                    anything_else = true;
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    write_element(c);
                    return;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    this->current_attribute_name = std::make_shared<UnicodeString>();
                    this->current_attribute_name->push_back(c);
                    SwitchToState(attribute_name_state);
                }
            }
            break;

        case attribute_name_state:
            {
                bool anything_else = false;

                if (c >= 'A' && c <= 'Z')
                {
                    this->current_attribute_name->push_back(c + 0x0020);
                }
                else switch (c)
                {
                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x0020:
                    SwitchToState(after_attribute_name_state);
                    break;

                case 0x002F:
                    SwitchToState(self_closing_start_tag_state);
                    break;

                case 0x003D:
                    SwitchToState(before_attribute_value_state);
                    break;

                case 0x003E:
                    SwitchToState(data_state);
                    EmitCurrentTagToken();
                    break;

                case 0x0000:
                    ParseError(c);
                    this->current_attribute_name->push_back(0xFFFD);
                    break;

                case 0x0022:
                case 0x0027:
                case 0x003C:
                    ParseError(c);
                    anything_else = true;
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    write_element(c);
                    return;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                    this->current_attribute_name->push_back(c);
            }
            break;

        case after_attribute_name_state:
            {
                bool anything_else = false;

                if (c >= 'A' && c <= 'Z')
                {
                    this->current_attribute_name = std::make_shared<UnicodeString>();
                    this->current_attribute_name->push_back(c + 0x0020);
                    SwitchToState(attribute_name_state);
                }
                else switch (c)
                {
                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x0020:
                    break;

                case 0x002F:
                    SwitchToState(self_closing_start_tag_state);
                    break;

                case 0x003D:
                    SwitchToState(before_attribute_value_state);
                    break;

                case 0x003E:
                    SwitchToState(data_state);
                    EmitCurrentTagToken();
                    break;

                case 0x0000:
                    ParseError(c);
                    this->current_attribute_name = std::make_shared<UnicodeString>();
                    this->current_attribute_name->push_back(0xFFFD);
                    SwitchToState(attribute_name_state);
                    break;

                case 0x0022:
                case 0x0027:
                case 0x003C:
                    ParseError(c);
                    anything_else = true;
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    write_element(c);
                    return;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    this->current_attribute_name = std::make_shared<UnicodeString>();
                    this->current_attribute_name->push_back(c);
                    SwitchToState(attribute_name_state);
                }
            }
            break;

        case before_attribute_value_state:
            {
                bool anything_else = false;

                switch (c)
                {
                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x0020:
                    break;

                case 0x0022:
                    SwitchToState(attribute_value_double_quoted_state);
                    break;

                case 0x0026:
                    SwitchToState(attribute_value_unquoted_state);
                    write_element(c);
                    return;

                case 0x0027:
                    SwitchToState(attribute_value_single_quoted_state);
                    break;

                case 0x0000:
                    ParseError(c);
                    this->current_attribute->second->push_back(0xFFFD);
                    SwitchToState(attribute_value_unquoted_state);
                    break;

                case 0x003E:
                    ParseError(c);
                    SwitchToState(data_state);
                    EmitCurrentTagToken();
                    break;

                case 0x003C:
                case 0x003D:
                case 0x0060:
                    ParseError(c);
                    anything_else = true;
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    write_element(c);
                    return;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    this->current_attribute->second->push_back(c);
                    SwitchToState(attribute_value_unquoted_state);
                }
            }
            break;

        case attribute_value_double_quoted_state:
            {
                switch (c)
                {
                case 0x0022:
                    SwitchToState(after_attribute_value_quoted_state);
                    break;

                case 0x0026:
                    SwitchToState(character_reference_in_attribute_value_state);
                    this->use_additional_allowed_character = true;
                    this->additional_allowed_character = 0x0022;
                    break;

                case 0x0000:
                    ParseError(c);
                    this->current_attribute->second->push_back(0xFFFD);
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    write_element(c);
                    return;

                default:
                    this->current_attribute->second->push_back(c);
                    break;
                }
            }
            break;

        case attribute_value_single_quoted_state:
            {
                switch (c)
                {
                case 0x0027:
                    SwitchToState(after_attribute_value_quoted_state);
                    break;

                case 0x0026:
                    SwitchToState(character_reference_in_attribute_value_state);
                    this->use_additional_allowed_character = true;
                    this->additional_allowed_character = 0x0027;
                    break;

                case 0x0000:
                    ParseError(c);
                    this->current_attribute->second->push_back(0xFFFD);
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    write_element(c);
                    return;

                default:
                    this->current_attribute->second->push_back(c);
                    break;
                }
            }
            break;

        case attribute_value_unquoted_state:
            {
                bool anything_else = false;

                switch (c)
                {
                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x0020:
                    SwitchToState(before_attribute_name_state);
                    break;

                case 0x0026:
                    SwitchToState(character_reference_in_attribute_value_state);
                    this->use_additional_allowed_character = true;
                    this->additional_allowed_character = 0x003E;
                    break;

                case 0x003E:
                    SwitchToState(data_state);
                    EmitCurrentTagToken();
                    break;

                case 0x0000:
                    ParseError(c);
                    this->current_attribute->second->push_back(0xFFFD);
                    break;

                case 0x0022:
                case 0x0027:
                case 0x003C:
                case 0x003D:
                case 0x0060:
                    ParseError(c);
                    anything_else = true;
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    write_element(c);
                    return;

                default:
                    anything_else = true;
                    break;
                }

                if (anything_else)
                {
                    this->current_attribute->second->push_back(c);
                }
            }
            break;

        case character_reference_in_attribute_value_state:
            {
                this->char_ref_frame.write_element(c);

                if (this->char_ref_frame.in_progress())
                    break;

                if (this->character_reference->size() == 0)
                {
                    this->current_attribute->second->push_back(0x0026);
                }
                else
                {
                    for (UnicodeString::iterator it = this->character_reference->begin(); it != this->character_reference->end(); it++)
                        this->current_attribute->second->push_back(*it);
                }

                SwitchToState(this->attribute_value_state);

                consume_leftovers();
                return;
            }
            break;

        case after_attribute_value_quoted_state:
            {
                switch (c)
                {
                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x0020:
                    SwitchToState(before_attribute_name_state);
                    break;

                case 0x002F:
                    SwitchToState(self_closing_start_tag_state);
                    break;

                case 0x003E:
                    SwitchToState(data_state);
                    EmitCurrentTagToken();
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    write_element(c);
                    return;

                default:
                    ParseError(c);
                    SwitchToState(before_attribute_name_state);
                    write_element(c);
                    return;
                }
            }
            break;

        case self_closing_start_tag_state:
            {
                switch (c)
                {
                case 0x003E:
                    this->current_tag_token->self_closing = true;
                    SwitchToState(data_state);
                    EmitCurrentTagToken();
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    write_element(c);
                    return;

                default:
                    ParseError(c);
                    SwitchToState(before_attribute_name_state);
                    write_element(c);
                    return;
                }
            }
            break;

        case bogus_comment_state:
            {
                switch (c)
                {
                case 0x003E:
                    Emit(this->comment_token);
                    this->comment_token = 0;
                    SwitchToState(data_state);
                    break;

                case EOF:
                    Emit(this->comment_token);
                    this->comment_token = 0;
                    SwitchToState(data_state);
                    write_element(c);
                    return;

                case 0x0000:
                    this->comment_token->data->push_back(0xFFFD);
                    break;

                default:
                    this->comment_token->data->push_back(c);
                    break;
                }

                // http://www.whatwg.org/specs/web-apps/current-work/multipage/tokenization.html#bogus-comment-state 12.2.4.44
                // $ NYI: (If the comment was started by the end of the file (EOF), the token is empty. Similarly, the token is empty if it was generated by the string "<!>".)
            }
            break;

        case markup_declaration_open_state:
            {
                this->markup_declaration_open->write_element(c);

                if (this->markup_declaration_open->size() != 2)
                    break;

                if (equals<UnicodeString, false>(this->markup_declaration_open.get(), Html::globals->markup_declaration_comment.get()))
                {
                    this->comment_token = std::make_shared<CommentToken>();
                    SwitchToState(comment_start_state_state);
                }
                else
                {
                    SwitchToState(markup_declaration_open_2_state);
                }
            }
            break;

        case markup_declaration_open_2_state:
            {
                this->markup_declaration_open->write_element(c);

                if (this->markup_declaration_open->size() != 7)
                    break;

                if (equals<UnicodeString, false>(this->markup_declaration_open.get(), Html::globals->markup_declaration_doctype.get()))
                {
                    SwitchToState(doctype_state);
                }
                else if (equals<UnicodeString, true>(this->markup_declaration_open.get(), Html::globals->markup_declaration_cdata.get()))
                {
                    SwitchToState(cdata_section_state);
                }
                else
                {
                    ParseError("unexpected markup_declaration_open");
                    SwitchToState(bogus_comment_state);
                    this->comment_token = std::make_shared<CommentToken>();

                    // $ NYI: The next character that is consumed, if any, is the first character that will be in the comment.
                    // not sure if the spec is right - if that's the case it would be this->markup_declaration_open[0] even if
                    // bogus_comment_state would not otherwise not have put it in the comment?

                    for (UnicodeString::iterator it = this->markup_declaration_open->begin(); it != this->markup_declaration_open->end(); it++)
                        this->write_element(*it);

                    return;
                }
            }
            break;

        case comment_start_state_state:
            {
                switch (c)
                {
                case 0x002D:
                    SwitchToState(comment_start_dash_state);
                    break;

                case 0x0000:
                    ParseError(c);
                    this->comment_token->data->push_back(0xFFFD);
                    SwitchToState(comment_state);
                    break;

                case 0x003E:
                    ParseError(c);
                    SwitchToState(data_state);
                    Emit(this->comment_token);
                    this->comment_token = 0;
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    Emit(this->comment_token);
                    this->comment_token = 0;
                    write_element(c);
                    return;

                default:
                    this->comment_token->data->push_back(c);
                    SwitchToState(comment_state);
                    break;
                }
            }
            break;

        case comment_start_dash_state:
            {
                switch (c)
                {
                case 0x002D:
                    SwitchToState(comment_end_state);
                    break;

                case 0x0000:
                    ParseError(c);
                    this->comment_token->data->push_back(0x002D);
                    this->comment_token->data->push_back(0xFFFD);
                    SwitchToState(comment_state);
                    break;

                case 0x003E:
                    ParseError(c);
                    SwitchToState(data_state);
                    Emit(this->comment_token);
                    this->comment_token = 0;
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    Emit(this->comment_token);
                    this->comment_token = 0;
                    write_element(c);
                    return;

                default:
                    this->comment_token->data->push_back(0x002D);
                    this->comment_token->data->push_back(c);
                    SwitchToState(comment_state);
                    break;
                }
            }
            break;

        case comment_state:
            {
                switch (c)
                {
                case 0x002D:
                    SwitchToState(comment_end_dash_state);
                    break;

                case 0x0000:
                    ParseError(c);
                    this->comment_token->data->push_back(0xFFFD);
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    Emit(this->comment_token);
                    this->comment_token = 0;
                    write_element(c);
                    return;

                default:
                    this->comment_token->data->push_back(c);
                    break;
                }
            }
            break;

        case comment_end_dash_state:
            {
                switch (c)
                {
                case 0x002D:
                    SwitchToState(comment_end_state);
                    break;

                case 0x0000:
                    ParseError(c);
                    this->comment_token->data->push_back(0x002D);
                    this->comment_token->data->push_back(0xFFFD);
                    SwitchToState(comment_state);
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    Emit(this->comment_token);
                    this->comment_token = 0;
                    write_element(c);
                    return;

                default:
                    this->comment_token->data->push_back(0x002D);
                    this->comment_token->data->push_back(c);
                    SwitchToState(comment_state);
                    break;
                }
            }
            break;

        case comment_end_state:
            {
                switch (c)
                {
                case 0x003E:
                    SwitchToState(data_state);
                    Emit(this->comment_token);
                    this->comment_token = 0;
                    break;

                case 0x0000:
                    ParseError(c);
                    this->comment_token->data->push_back(0x002D);
                    this->comment_token->data->push_back(0x002D);
                    this->comment_token->data->push_back(0xFFFD);
                    SwitchToState(comment_state);
                    break;

                case 0x0021:
                    ParseError(c);
                    SwitchToState(comment_end_bang_state);
                    break;

                case 0x002D:
                    ParseError(c);
                    this->comment_token->data->push_back(0x002D);
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    Emit(this->comment_token);
                    this->comment_token = 0;
                    write_element(c);
                    return;

                default:
                    ParseError(c);
                    this->comment_token->data->push_back(0x002D);
                    this->comment_token->data->push_back(0x002D);
                    this->comment_token->data->push_back(c);
                    SwitchToState(comment_state);
                    break;
                }
            }
            break;

        case comment_end_bang_state:
            {
                switch (c)
                {
                case 0x002D:
                    this->comment_token->data->push_back(0x002D);
                    this->comment_token->data->push_back(0x002D);
                    this->comment_token->data->push_back(0x0021);
                    SwitchToState(comment_end_dash_state);
                    break;

                case 0x003E:
                    SwitchToState(data_state);
                    Emit(this->comment_token);
                    this->comment_token = 0;
                    break;

                case 0x0000:
                    ParseError(c);
                    this->comment_token->data->push_back(0x002D);
                    this->comment_token->data->push_back(0x002D);
                    this->comment_token->data->push_back(0x0021);
                    SwitchToState(comment_state);
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    Emit(this->comment_token);
                    this->comment_token = 0;
                    write_element(c);
                    return;

                default:
                    ParseError(c);
                    this->comment_token->data->push_back(0x002D);
                    this->comment_token->data->push_back(0x002D);
                    this->comment_token->data->push_back(0x0021);
                    this->comment_token->data->push_back(c);
                    SwitchToState(comment_state);
                    break;
                }
            }
            break;

        case doctype_state:
            {
                switch (c)
                {
                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x0020:
                    SwitchToState(before_doctype_name_state);
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);

                    this->doctype_token = std::make_shared<DocTypeToken>();
                    this->doctype_token->force_quirks = true;
                    Emit(this->doctype_token);
                    this->doctype_token = 0;

                    write_element(c);
                    return;

                default:
                    ParseError(c);
                    SwitchToState(before_doctype_name_state);
                    write_element(c);
                    return;
                }
            }
            break;

        case before_doctype_name_state:
            {
                if (c >= 'A' && c <= 'Z')
                {
                    this->doctype_token = std::make_shared<DocTypeToken>();
                    this->doctype_token->name = std::make_shared<UnicodeString>();
                    this->doctype_token->name->push_back(c + 0x0020);

                    SwitchToState(doctype_name_state);
                }
                else switch (c)
                {
                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x0020:
                    break;

                case 0x0000:
                    ParseError(c);

                    this->doctype_token = std::make_shared<DocTypeToken>();
                    this->doctype_token->name = std::make_shared<UnicodeString>();
                    this->doctype_token->name->push_back(0xFFFD);

                    SwitchToState(doctype_name_state);
                    break;

                case 0x003E:
                    ParseError(c);

                    this->doctype_token = std::make_shared<DocTypeToken>();
                    this->doctype_token->force_quirks = true;

                    SwitchToState(data_state);
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);

                    this->doctype_token = std::make_shared<DocTypeToken>();
                    this->doctype_token->force_quirks = true;

                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    write_element(c);
                    return;

                default:
                    this->doctype_token = std::make_shared<DocTypeToken>();
                    this->doctype_token->name = std::make_shared<UnicodeString>();
                    this->doctype_token->name->push_back(c);
                    SwitchToState(doctype_name_state);
                    break;
                }
            }
            break;

        case doctype_name_state:
            {
                if (c >= 'A' && c <= 'Z')
                {
                    this->doctype_token->name->push_back(c + 0x0020);
                }
                else switch (c)
                {
                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x0020:
                    SwitchToState(after_doctype_name_state);
                    break;

                case 0x003E:
                    SwitchToState(data_state);
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    break;

                case 0x0000:
                    ParseError(c);
                    this->doctype_token->name->push_back(0xFFFD);
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    this->doctype_token->force_quirks = true;
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    write_element(c);
                    return;

                default:
                    this->doctype_token->name->push_back(c);
                    break;
                }
            }
            break;

        case after_doctype_name_state:
            {
                switch (c)
                {
                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x0020:
                    break;

                case 0x003E:
                    SwitchToState(data_state);
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    this->doctype_token->force_quirks = true;
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    write_element(c);
                    return;

                default:
                    {
                        this->after_doctype_name->clear();
                        this->after_doctype_name->reserve(6);
                        this->after_doctype_name->push_back(c);

                        SwitchToState(after_doctype_name_2_state);
                    }
                    break;
                }
            }
            break;

        case after_doctype_name_2_state:
            {
                this->after_doctype_name->write_element(c);

                if (this->after_doctype_name->size() != 6)
                    break;

                if (equals<UnicodeString, false>(this->after_doctype_name.get(), Html::globals->after_doctype_public_keyword.get()))
                {
                    SwitchToState(after_doctype_public_keyword_state);
                }
                else if (equals<UnicodeString, false>(this->after_doctype_name.get(), Html::globals->after_doctype_system_keyword.get()))
                {
                    SwitchToState(after_doctype_system_keyword_state);
                }
                else
                {
                    ParseError("unexpected after_doctype_name");
                    this->doctype_token->force_quirks = true;
                    SwitchToState(bogus_doctype_state);

                    for (UnicodeString::iterator it = this->after_doctype_name->begin(); it != this->after_doctype_name->end(); it++)
                        this->write_element(*it);

                    return;
                }
            }
            break;

        case after_doctype_public_keyword_state:
            {
                switch (c)
                {
                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x0020:
                    SwitchToState(before_doctype_public_identifier_state);
                    break;

                case 0x0022:
                    ParseError(c);
                    this->doctype_token->public_identifier = std::make_shared<UnicodeString>();
                    SwitchToState(doctype_public_identifier_double_quoted_state);
                    break;

                case 0x0027:
                    ParseError(c);
                    this->doctype_token->public_identifier = std::make_shared<UnicodeString>();
                    SwitchToState(doctype_public_identifier_single_quoted_state);
                    break;

                case 0x003E:
                    ParseError(c);
                    this->doctype_token->force_quirks = true;
                    SwitchToState(data_state);
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    this->doctype_token->force_quirks = true;
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    write_element(c);
                    return;

                default:
                    ParseError(c);
                    this->doctype_token->force_quirks = true;
                    SwitchToState(bogus_doctype_state);
                    break;
                }
            }
            break;

        case before_doctype_public_identifier_state:
            {
                switch (c)
                {
                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x0020:
                    break;

                case 0x0022:
                    this->doctype_token->public_identifier = std::make_shared<UnicodeString>();
                    SwitchToState(doctype_public_identifier_double_quoted_state);
                    break;

                case 0x0027:
                    this->doctype_token->public_identifier = std::make_shared<UnicodeString>();
                    SwitchToState(doctype_public_identifier_single_quoted_state);
                    break;

                case 0x003E:
                    ParseError(c);
                    this->doctype_token->force_quirks = true;
                    SwitchToState(data_state);
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    this->doctype_token->force_quirks = true;
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    write_element(c);
                    return;

                default:
                    ParseError(c);
                    this->doctype_token->force_quirks = true;
                    SwitchToState(bogus_doctype_state);
                    break;
                }
            }
            break;

        case doctype_public_identifier_double_quoted_state:
            {
                switch (c)
                {
                case 0x0022:
                    SwitchToState(after_doctype_public_identifier_state);
                    break;

                case 0x0000:
                    ParseError(c);
                    this->doctype_token->public_identifier->push_back(0xFFFD);
                    break;

                case 0x003E:
                    ParseError(c);
                    this->doctype_token->force_quirks = true;
                    SwitchToState(data_state);
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    this->doctype_token->force_quirks = true;
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    write_element(c);
                    return;

                default:
                    this->doctype_token->public_identifier->push_back(c);
                    break;
                }
            }
            break;

        case doctype_public_identifier_single_quoted_state:
            {
                switch (c)
                {
                case 0x0027:
                    SwitchToState(after_doctype_public_identifier_state);
                    break;

                case 0x0000:
                    ParseError(c);
                    this->doctype_token->public_identifier->push_back(0xFFFD);
                    break;

                case 0x003E:
                    ParseError(c);
                    this->doctype_token->force_quirks = true;
                    SwitchToState(data_state);
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    this->doctype_token->force_quirks = true;
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    write_element(c);
                    return;

                default:
                    this->doctype_token->public_identifier->push_back(c);
                    break;
                }
            }
            break;

        case after_doctype_public_identifier_state:
            {
                switch (c)
                {
                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x0020:
                    SwitchToState(between_doctype_public_and_system_identifiers_state);
                    break;

                case 0x003E:
                    SwitchToState(data_state);
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    break;

                case 0x0022:
                    ParseError(c);
                    this->doctype_token->public_identifier = std::make_shared<UnicodeString>();
                    SwitchToState(doctype_public_identifier_double_quoted_state);
                    break;

                case 0x0027:
                    ParseError(c);
                    this->doctype_token->public_identifier = std::make_shared<UnicodeString>();
                    SwitchToState(doctype_public_identifier_single_quoted_state);
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    this->doctype_token->force_quirks = true;
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    write_element(c);
                    return;

                default:
                    ParseError(c);
                    this->doctype_token->force_quirks = true;
                    SwitchToState(bogus_doctype_state);
                    break;
                }
            }
            break;

        case between_doctype_public_and_system_identifiers_state:
            {
                switch (c)
                {
                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x0020:
                    break;

                case 0x003E:
                    SwitchToState(data_state);
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    break;

                case 0x0022:
                    this->doctype_token->public_identifier = std::make_shared<UnicodeString>();
                    SwitchToState(doctype_public_identifier_double_quoted_state);
                    break;

                case 0x0027:
                    this->doctype_token->public_identifier = std::make_shared<UnicodeString>();
                    SwitchToState(doctype_public_identifier_single_quoted_state);
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    this->doctype_token->force_quirks = true;
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    write_element(c);
                    return;

                default:
                    ParseError(c);
                    this->doctype_token->force_quirks = true;
                    SwitchToState(bogus_doctype_state);
                    break;
                }
            }
            break;

        case after_doctype_system_keyword_state:
            {
                switch (c)
                {
                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x0020:
                    SwitchToState(before_doctype_system_identifier_state);
                    break;

                case 0x0022:
                    ParseError(c);
                    this->doctype_token->system_identifier = std::make_shared<UnicodeString>();
                    SwitchToState(doctype_system_identifier_double_quoted_state);
                    break;

                case 0x0027:
                    ParseError(c);
                    this->doctype_token->system_identifier = std::make_shared<UnicodeString>();
                    SwitchToState(doctype_system_identifier_single_quoted_state);
                    break;

                case 0x003E:
                    ParseError(c);
                    this->doctype_token->force_quirks = true;
                    SwitchToState(data_state);
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    this->doctype_token->force_quirks = true;
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    write_element(c);
                    return;

                default:
                    ParseError(c);
                    this->doctype_token->force_quirks = true;
                    SwitchToState(bogus_doctype_state);
                    break;
                }
            }
            break;

        case before_doctype_system_identifier_state:
            {
                switch (c)
                {
                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x0020:
                    break;

                case 0x0022:
                    this->doctype_token->system_identifier = std::make_shared<UnicodeString>();
                    SwitchToState(doctype_system_identifier_double_quoted_state);
                    break;

                case 0x0027:
                    this->doctype_token->system_identifier = std::make_shared<UnicodeString>();
                    SwitchToState(doctype_system_identifier_single_quoted_state);
                    break;

                case 0x003E:
                    ParseError(c);
                    this->doctype_token->force_quirks = true;
                    SwitchToState(data_state);
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    this->doctype_token->force_quirks = true;
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    write_element(c);
                    return;

                default:
                    ParseError(c);
                    this->doctype_token->force_quirks = true;
                    SwitchToState(bogus_doctype_state);
                    break;
                }
            }
            break;

        case doctype_system_identifier_double_quoted_state:
            {
                switch (c)
                {
                case 0x0022:
                    SwitchToState(after_doctype_system_identifier_state);
                    break;

                case 0x0000:
                    ParseError(c);
                    this->doctype_token->system_identifier->push_back(0xFFFD);
                    break;

                case 0x003E:
                    ParseError(c);
                    this->doctype_token->force_quirks = true;
                    SwitchToState(data_state);
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    this->doctype_token->force_quirks = true;
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    write_element(c);
                    return;

                default:
                    this->doctype_token->system_identifier->push_back(c);
                    break;
                }
            }
            break;

        case doctype_system_identifier_single_quoted_state:
            {
                switch (c)
                {
                case 0x0027:
                    SwitchToState(after_doctype_system_identifier_state);
                    break;

                case 0x0000:
                    ParseError(c);
                    this->doctype_token->system_identifier->push_back(0xFFFD);
                    break;

                case 0x003E:
                    ParseError(c);
                    this->doctype_token->force_quirks = true;
                    SwitchToState(data_state);
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    this->doctype_token->force_quirks = true;
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    write_element(c);
                    return;

                default:
                    this->doctype_token->system_identifier->push_back(c);
                    break;
                }
            }
            break;

        case after_doctype_system_identifier_state:
            {
                switch (c)
                {
                case 0x0009:
                case 0x000A:
                case 0x000C:
                case 0x0020:
                    break;

                case 0x003E:
                    SwitchToState(data_state);
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    break;

                case EOF:
                    ParseError(c);
                    SwitchToState(data_state);
                    this->doctype_token->force_quirks = true;
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    write_element(c);
                    return;

                default:
                    ParseError(c);
                    SwitchToState(bogus_doctype_state);
                    break;
                }
            }
            break;

        case bogus_doctype_state:
            {
                switch (c)
                {
                case 0x003E:
                    SwitchToState(data_state);
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    break;

                case EOF:
                    SwitchToState(data_state);
                    Emit(this->doctype_token);
                    this->doctype_token = 0;
                    write_element(c);
                    return;

                default:
                    break;
                }
            }
            break;

        case cdata_section_state:
            {
                switch (c)
                {
                case 0x005D:
                    SwitchToState(cdata_section_2_state);
                    break;

                case EOF:
                    SwitchToState(data_state);
                    write_element(c);
                    return;

                default:
                    EmitCharacter(c);
                    break;
                }
            }
            break;

        case cdata_section_2_state:
            {
                switch (c)
                {
                case 0x005D:
                    SwitchToState(cdata_section_3_state);
                    break;

                default:
                    EmitCharacter(0x005D);
                    SwitchToState(cdata_section_state);
                    write_element(c);
                    return;
                }
            }
            break;

        case cdata_section_3_state:
            {
                switch (c)
                {
                case 0x003E:
                    SwitchToState(data_state);
                    break;

                default:
                    EmitCharacter(0x005D);
                    EmitCharacter(0x005D);
                    SwitchToState(cdata_section_state);
                    write_element(c);
                    return;
                }
            }
            break;

        default:
            throw FatalError("Tokenizer::write_element unexpected state");
        }
    }

    void Tokenizer::ParseError(Codepoint c)
    {
        char error[0x100];
        sprintf_s(error, "codepoint: 0x%04X '%c'", c, c);

        ParseError(error);
    }

    void Tokenizer::ParseError(const char* error)
    {
        char state_string[0x40];

        switch (this->get_state())
        {
#define CASE(e) \
        case e: \
            strcpy_s(state_string, #e); \
            break

            CASE(data_state);
            CASE(character_reference_in_data_state);
            CASE(RCDATA_state);
            CASE(character_reference_in_RCDATA_state);
            CASE(RAWTEXT_state);
            CASE(script_data_state);
            CASE(PLAINTEXT_state);
            CASE(tag_open_state);
            CASE(end_tag_open_state);
            CASE(tag_name_state);
            CASE(RCDATA_less_than_sign_state);
            CASE(RCDATA_end_tag_open_state);
            CASE(RCDATA_end_tag_name_state);
            CASE(RAWTEXT_less_than_sign_state);
            CASE(RAWTEXT_end_tag_open_state);
            CASE(RAWTEXT_end_tag_name_state);
            CASE(script_data_less_than_sign_state);
            CASE(script_data_end_tag_open_state);
            CASE(script_data_end_tag_name_state);
            CASE(script_data_escape_start_state);
            CASE(script_data_escape_start_dash_state);
            CASE(script_data_escaped_state);
            CASE(script_data_escaped_dash_state);
            CASE(script_data_escaped_dash_dash_state);
            CASE(script_data_escaped_less_than_sign_state);
            CASE(script_data_escaped_end_tag_open_state);
            CASE(script_data_escaped_end_tag_name_state);
            CASE(script_data_double_escape_start_state);
            CASE(script_data_double_escaped_state);
            CASE(script_data_double_escaped_dash_state);
            CASE(script_data_double_escaped_dash_dash_state);
            CASE(script_data_double_escaped_less_than_sign_state);
            CASE(script_data_double_escape_end_state);
            CASE(before_attribute_name_state);
            CASE(attribute_name_state);
            CASE(after_attribute_name_state);
            CASE(before_attribute_value_state);
            CASE(attribute_value_double_quoted_state);
            CASE(attribute_value_single_quoted_state);
            CASE(attribute_value_unquoted_state);
            CASE(character_reference_in_attribute_value_state);
            CASE(after_attribute_value_quoted_state);
            CASE(self_closing_start_tag_state);
            CASE(bogus_comment_state);
            CASE(markup_declaration_open_state);
            CASE(markup_declaration_open_2_state);
            CASE(comment_start_state_state);
            CASE(comment_start_dash_state);
            CASE(comment_state);
            CASE(comment_end_dash_state);
            CASE(comment_end_state);
            CASE(comment_end_bang_state);
            CASE(doctype_state);
            CASE(before_doctype_name_state);
            CASE(doctype_name_state);
            CASE(after_doctype_name_state);
            CASE(after_doctype_name_2_state);
            CASE(after_doctype_public_keyword_state);
            CASE(after_doctype_system_keyword_state);
            CASE(bogus_doctype_state);
            CASE(before_doctype_public_identifier_state);
            CASE(doctype_public_identifier_double_quoted_state);
            CASE(doctype_public_identifier_single_quoted_state);
            CASE(after_doctype_public_identifier_state);
            CASE(between_doctype_public_and_system_identifiers_state);
            CASE(before_doctype_system_identifier_state);
            CASE(doctype_system_identifier_double_quoted_state);
            CASE(doctype_system_identifier_single_quoted_state);
            CASE(after_doctype_system_identifier_state);
            CASE(cdata_section_state);
            CASE(cdata_section_2_state);
            CASE(cdata_section_3_state);

#undef CASE

        default:
            sprintf_s(state_string, "%d", this->get_state());
            break;
        }

        char full_error[0x100];
        sprintf_s(full_error, "state: %s, %s", state_string, error);

        this->parser->ParseError(full_error);
    }

    void Tokenizer::ParseError(Codepoint c, const char* error)
    {
        char full_error[0x100];
        sprintf_s(full_error, "codepoint: 0x%04X '%c', %s", c, c, error);

        ParseError(full_error);
    }
}