// Copyright © 2013 Brian Spanton

#pragma once

#include "Html.Types.h"
#include "Html.Node.h"
#include "Html.Token.h"
#include "Basic.StreamFrame.h"
#include "Html.CharacterToken.h"
#include "Html.StartTagToken.h"
#include "Html.EndTagToken.h"
#include "Html.CommentToken.h"
#include "Html.DocTypeToken.h"
#include "Html.CharacterReferenceFrame.h"
#include "Html.EndOfFileToken.h"

namespace Html
{
    using namespace Basic;

    class Parser;

    class Tokenizer : public StateMachine, public UnitStream<Codepoint>, public std::enable_shared_from_this<Tokenizer>
    {
    private:
        std::shared_ptr<IStream<TokenRef> > output;
        Parser* parser;
        UnicodeStringRef character_reference;
        UnicodeStringRef character_reference_unconsume;
        std::shared_ptr<TagToken> current_tag_token;
        UnicodeStringRef temporary_buffer;
        UnicodeStringRef last_start_tag_name;
        UnicodeStringRef current_attribute_name;
        StringMap::iterator current_attribute;
        bool use_additional_allowed_character;
        Codepoint additional_allowed_character;
        TokenizerState attribute_value_state;
        std::shared_ptr<CommentToken> comment_token;
        UnicodeStringRef markup_declaration_open;
        std::shared_ptr<DocTypeToken> doctype_token;
        UnicodeStringRef after_doctype_name;
        std::shared_ptr<EndOfFileToken> eof_token;
        CharacterReferenceFrame char_ref_frame;

        void Emit(TokenRef token);
        void EmitCurrentTagToken();
        void EmitCharacter(Codepoint c);
        bool IsAppropriate(Token* token);
        void SwitchToState(TokenizerState state);
        void InsertAttribute();
        void ParseError(Codepoint c);
        void ParseError(const char* error);
        void ParseError(Codepoint c, const char* error);

    public:
        friend class TreeConstruction;

        Tokenizer(Parser* parser, std::shared_ptr<IStream<TokenRef> > output);

        virtual void IStream<Codepoint>::write_element(Codepoint element);
        virtual void IStream<Codepoint>::write_eof();
    };
}