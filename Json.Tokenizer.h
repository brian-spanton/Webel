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

    class Tokenizer : public StateMachine, public UnitStream<Codepoint>
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
            utf16_surrogate_state,
            number_state,
            fraction_state,
            exponent_state,
            done_state = Succeeded_State,
            error_state,
        };

        typedef std::unordered_map<UnicodeStringRef, std::shared_ptr<Token> > LiteralMap;

        static LiteralMap literal_map; // $ why is this static instead of on globals

        std::shared_ptr<IStream<std::shared_ptr<Token> > > output;
        uint8 matched = 0;
        LiteralMap::iterator literal_it;
        UnicodeStringRef string;
        int64 sign = 0;
        DecNumberStream<Codepoint, uint64> dec_number_stream;
        HexNumberStream<Codepoint, uint64> hex_number_stream;
        long double number = 0;
        uint64 whole = 0;
        uint64 fraction = 0;
        uint64 exponent = 0;
        bool utf16_surrogate = false;
        Codepoint utf16_lead_surrogate = 0;

        void Emit(std::shared_ptr<Token> token);
        void handle_error(const char* error);

    public:
        static void InitializeStatics();

        Tokenizer(std::shared_ptr<IStream<std::shared_ptr<Token> > > output);

        virtual void IStream<Codepoint>::write_element(Codepoint element);
    };
}