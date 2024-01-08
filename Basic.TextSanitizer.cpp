// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Basic.TextSanitizer.h"
#include "Basic.Globals.h"

namespace Basic
{
    bool TextSanitizer::white_space(Codepoint codepoint)
    {
        if (codepoint >= 0x100)
            return false;

        if (Basic::globals->sanitizer_white_space[codepoint])
            return true;

        return false;
    }

    void TextSanitizer::Initialize(IStream<Codepoint>* destination)
    {
        this->state = State::before_first_word_state;
        this->destination = destination;
    }

    void TextSanitizer::write_element(Codepoint codepoint)
    {
        switch (this->state)
        {
        case State::before_first_word_state:
            if (!white_space(codepoint))
            {
                this->destination->write_element(codepoint);
                this->state = State::in_word_state;
            }
            break;

        case State::in_word_state:
            if (white_space(codepoint))
            {
                this->state = State::before_next_word_state;
            }
            else
            {
                this->destination->write_element(codepoint);
            }
            break;

        case State::before_next_word_state:
            if (!white_space(codepoint))
            {
                this->destination->write_element(0x0020);
                this->destination->write_element(codepoint);
                this->state = State::in_word_state;
            }
            break;
        }
    }
}