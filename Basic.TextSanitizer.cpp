// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.TextSanitizer.h"
#include "Basic.Globals.h"

namespace Basic
{
    bool TextSanitizer::white_space(Codepoint c)
    {
        if (c >= 0x100)
            return false;

        if (Basic::globals->sanitizer_white_space[c])
            return true;

        return false;
    }

    void TextSanitizer::Initialize(IStream<Codepoint>* destination)
    {
        this->state = State::before_first_word_state;
        this->destination = destination;
    }

    void TextSanitizer::write_element(Codepoint c)
    {
        switch (this->state)
        {
        case State::before_first_word_state:
            if (!white_space(c))
            {
                this->destination->write_element(c);
                this->state = State::in_word_state;
            }
            break;

        case State::in_word_state:
            if (white_space(c))
            {
                this->state = State::before_next_word_state;
            }
            else
            {
                this->destination->write_element(c);
            }
            break;

        case State::before_next_word_state:
            if (!white_space(c))
            {
                this->destination->write_element(0x0020);
                this->destination->write_element(c);
                this->state = State::in_word_state;
            }
            break;
        }
    }
}