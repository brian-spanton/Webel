// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Html.InputStreamPreprocessor.h"
#include "Html.Globals.h"
#include "Html.Types.h"
#include "Html.StartTagToken.h"
#include "Html.EndTagToken.h"
#include "Html.CharacterToken.h"
#include "Html.EndOfFileToken.h"
#include "Html.Parser.h"

namespace Html
{
    using namespace Basic;

    InputStreamPreprocessor::InputStreamPreprocessor(Parser* parser, std::shared_ptr<IStream<Codepoint> > output) :
        state(State::bom_state),
        parser(parser),
        output(output)
    {
    }

    void InputStreamPreprocessor::write_element(Codepoint c)
    {
        switch (this->state)
        {
        case State::bom_state:
            {
                this->state = State::normal_state;

                if (c != 0xFEFF)
                {
                    write_element(c);
                    return;
                }
            }
            break;

        case State::normal_state:
            {
                if (!IsValid(c))
                    this->parser->ParseError("invalid char in input stream");

                if (c == 0x000D)
                    c = 0x000A;

                if (c == 0x000A)
                    this->state = State::ignore_lf_state;

                this->output->write_element(c);
            }
            break;

        case State::ignore_lf_state:
            {
                if (!IsValid(c))
                    this->parser->ParseError("invalid char in input stream");

                if (c == 0x000D)
                    c = 0x000A;

                if (c != 0x000A)
                {
                    this->state = State::normal_state;

                    this->output->write_element(c);
                }
            }
            break;

        default:
            throw FatalError("Html::InputStreamPreprocessor::write_element enexpected state", 0);
        }
    }

    bool InputStreamPreprocessor::IsValid(Codepoint c)
    {
        if (c >= 0x0001 && c <= 0x0008)
            return false;

        if (c >= 0x000E && c <= 0x001F)
            return false;

        if (c >= 0x007F && c <= 0x009F)
            return false;

        if (c >= 0xFDD0 && c <= 0xFDEF)
            return false;

        switch (c)
        {
        case 0x000B:
        case 0xFFFE:
        case 0xFFFF:
        case 0x1FFFE:
        case 0x1FFFF:
        case 0x2FFFE:
        case 0x2FFFF:
        case 0x3FFFE:
        case 0x3FFFF:
        case 0x4FFFE:
        case 0x4FFFF:
        case 0x5FFFE:
        case 0x5FFFF:
        case 0x6FFFE:
        case 0x6FFFF:
        case 0x7FFFE:
        case 0x7FFFF:
        case 0x8FFFE:
        case 0x8FFFF:
        case 0x9FFFE:
        case 0x9FFFF:
        case 0xAFFFE:
        case 0xAFFFF:
        case 0xBFFFE:
        case 0xBFFFF:
        case 0xCFFFE:
        case 0xCFFFF:
        case 0xDFFFE:
        case 0xDFFFF:
        case 0xEFFFE:
        case 0xEFFFF:
        case 0xFFFFE:
        case 0xFFFFF:
        case 0x10FFFE:
        case 0x10FFFF:
            return false;

        default:
            return true;
        }
    }
}