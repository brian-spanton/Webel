// Copyright © 2013 Brian Spanton

#pragma once

#include "Json.Types.h"
#include "Json.Text.h"
#include "Basic.DecNumberStream.h"
#include "Basic.HexNumberStream.h"

namespace Json
{
    using namespace Basic;

    class Parser;

    class Tokenizer : public Frame
    {
    private:
        enum State
        {
            start_state = Start_State,
            literal_state,
            token_state,
            string_state,
            escape_state,
            character_code_state,
            number_state,
            fraction_state,
            exponent_state,
            done_state = Succeeded_State,
            error_state,
        };

        typedef std::unordered_map<UnicodeString::Ref, Token::Ref> LiteralMap;

        static LiteralMap literal_map;

        Basic::Ref<IStream<Token::Ref> > output; // REF
        uint8 matched;
        LiteralMap::iterator literal_it;
        UnicodeString::Ref string; // REF
        int64 sign;
        Inline<DecNumberStream<Codepoint, uint64> > dec_number_stream;
        Inline<HexNumberStream<Codepoint, uint64> > hex_number_stream;
        long double number;
        uint64 whole;
        uint64 fraction;
        uint64 exponent;

        void Emit(Token::Ref token);
        void Error(const char* error);

    public:
        typedef Basic::Ref<Tokenizer, IProcess> Ref;

        static void InitializeStatics();

        void Initialize(IStream<Token::Ref>* output);
        virtual void IProcess::Process(IEvent* event, bool* yield);
    };
}