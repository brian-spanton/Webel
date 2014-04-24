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

    class Tokenizer : public IProcess
    {
    private:
        TokenizerState state;
        Basic::Ref<IStream<Token*> > output; // REF
        Parser* parser;
        UnicodeString::Ref character_reference; // REF
        UnicodeString::Ref character_reference_unconsume; // REF
        TagToken::Ref current_tag_token; // REF
        UnicodeString::Ref temporary_buffer; // REF
        StartTagToken::Ref last_start_tag; // REF
        UnicodeString::Ref current_attribute_name; // REF
        StringMap::iterator current_attribute;
        bool use_additional_allowed_character;
        Codepoint additional_allowed_character;
        TokenizerState attribute_value_state;
        CommentToken::Ref comment_token; // REF
        UnicodeString::Ref markup_declaration_open; // REF
        DocTypeToken::Ref doctype_token; // REF
        UnicodeString::Ref after_doctype_name; // REF
        Inline<EndOfFileToken> eof_token;
        Inline<CharacterReferenceFrame> char_ref_frame;
        Inline<StreamFrame<Codepoint> > stream_frame;

        void Emit(Token* token);
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

        typedef Basic::Ref<Tokenizer, IProcess> Ref;

        void Initialize(Parser* parser, IStream<Token*>* output);

        virtual void IProcess::Process(IEvent* event, bool* yield);
        virtual void IProcess::Process(IEvent* event);
        virtual bool IProcess::Pending();
        virtual bool IProcess::Succeeded();
        virtual bool IProcess::Failed();
    };
}