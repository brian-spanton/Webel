// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Http.MediaType.h"

namespace Http
{
    using namespace Basic;

    class MediaTypeStream : public UnitStream<Codepoint>
    {
    private:
        enum State
        {
            type_state,
            subtype_state,
            after_subtype_state,
            before_name_state,
            name_state,
            after_name_state,
            before_value_state,
            value_state,
            value_quoted_state,
            after_value_state,
            parse_error,
        };

        State state;
        MediaType* mediaType;
        UnicodeStringRef name;
        UnicodeStringRef value;

        void ParseError(Codepoint c);

    public:
        void Initialize(MediaType* mediaType);

        virtual void IStream<Codepoint>::write_element(Codepoint element);
        virtual void IStream<Codepoint>::write_eof();
    };
}