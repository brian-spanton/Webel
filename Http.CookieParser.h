// Copyright © 2013 Brian Spanton

#pragma once

#include "Http.Types.h"
#include "Basic.IProcess.h"

namespace Http
{
    using namespace Basic;

    class CookieParser : public UnitStream<Codepoint>
    {
    private:
        enum State
        {
            in_name_state,
            before_value_state,
            in_value_state,
            before_attribute_name_state,
            in_attribute_name_state,
            ignore_attribute_state,
            expires_state,
            expires_value_state,
            max_age_state,
            max_age_value_state,
            domain_state,
            domain_value_start_state,
            domain_value_state,
            path_state,
            path_value_start_state,
            path_value_state,
            secure_state,
            http_only_state,
            parse_error,
        };

        typedef StringMapCaseSensitive<State> StringMap;

        static StringMap attr_map;

        State state = State::in_name_state;
        Cookie* cookie = 0;
        StringMap::iterator attr;
        uint8 matched = 0;
        UnicodeStringRef node;

        void ParseError(Codepoint c);

    public:
        static void InitializeStatics();

        void Initialize(Cookie* cookie);

        virtual void IStream<Codepoint>::write_element(Codepoint element);
        virtual void IStream<Codepoint>::write_eof();
    };
}