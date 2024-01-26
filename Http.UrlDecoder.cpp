// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Http.UrlDecoder.h"
#include "Http.Globals.h"

namespace Http
{
    using namespace Basic;

    void UrlDecoder::Initialize(std::shared_ptr<IStream<byte> > destination)
    {
        this->state = State::normal_state;
        this->destination = destination;
    }

    void UrlDecoder::write_element(byte b)
    {
        switch(this->state)
        {
        case State::normal_state:
            {
                if (b == '+')
                {
                    this->destination->write_element(Http::globals->SP);
                }
                else if (b == '%')
                {
                    this->hex = 0;
                    this->state = State::hex1_state;
                }
                else
                {
                    this->destination->write_element(b);
                }
            }
            break;

        case State::hex1_state:
            {
                bool success = base_16(b, &hex);
                if (!success)
                {
                    this->state = State::hex1_error;
                }
                else
                {
                    this->state = State::hex2_state;
                }
            }
            break;

        case State::hex2_state:
            {
                byte digit_value;

                bool success = base_16(b, &digit_value);
                if (!success)
                {
                    state = State::hex2_error;
                }
                else
                {
                    this->hex *= 0x10;
                    this->hex += digit_value;

                    this->destination->write_element(this->hex);

                    state = State::normal_state;
                }
            }
            break;

        case State::hex1_error:
        case State::hex2_error:
            break;

        default:
            throw FatalError("Http", "UrlDecoder", "write_element", "unhandled state", this->state);
        }
    }

    void UrlDecoder::write_eof()
    {
        if (this->state != State::normal_state)
            Basic::LogDebug("Http", "UrlDecoder", "write_eof", "this->state != State::normal_state");
    }
}