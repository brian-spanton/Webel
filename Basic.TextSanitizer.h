// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"

namespace Basic
{
    class TextSanitizer : public UnitStream<Codepoint>
    {
    private:
        enum State
        {
            before_first_word_state,
            in_word_state,
            before_next_word_state,
        };

        State state;
        IStream<Codepoint>* destination;

        bool white_space(Codepoint c);

    public:
        void Initialize(IStream<Codepoint>* destination);

        virtual void IStream<Codepoint>::write_element(Codepoint element);
    };
}