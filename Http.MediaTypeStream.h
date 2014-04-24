// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Http.MediaType.h"

namespace Http
{
    using namespace Basic;

    class MediaTypeStream : public IStream<Codepoint>
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
        UnicodeString::Ref name; // REF
        UnicodeString::Ref value; // REF

        void ParseError(Codepoint c);

    public:
        typedef Basic::Ref<MediaTypeStream, IStream<Codepoint> > Ref;

        void Initialize(MediaType* mediaType);

        virtual void IStream<Codepoint>::Write(const Codepoint* elements, uint32 count);
        virtual void IStream<Codepoint>::WriteEOF();
    };
}