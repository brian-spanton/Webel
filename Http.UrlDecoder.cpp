// Copyright © 2013 Brian Spanton

#include "stdafx.h"
#include "Http.UrlDecoder.h"
#include "Http.Globals.h"

namespace Http
{
    using namespace Basic;

    void UrlDecoder::Initialize(IStream<byte>* destination)
    {
        this->state = State::normal_state;
        this->destination = destination;
    }

    void UrlDecoder::Write(const byte* elements, uint32 count)
    {
        for (uint32 i = 0; i < count; i++)
        {
            byte b = elements[i];
            switch(this->state)
            {
            case State::normal_state:
                {
                    if (b == '+')
                    {
                        b = Http::globals->SP;

                        this->destination->Write(&b, 1);
                    }
                    else if (b == '%')
                    {
                        this->hex = 0;
                        this->state = State::hex1_state;
                    }
                    else
                    {
                        this->destination->Write(&b, 1);
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

                        this->destination->Write(&this->hex, 1);

                        state = State::normal_state;
                    }
                }
                break;

            case State::hex1_error:
            case State::hex2_error:
                break;

            default:
                throw new Exception("UrlDecoder::Write unexpected state");
            }
        }
    }

    void UrlDecoder::WriteEOF()
    {
    }
}