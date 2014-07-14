// Copyright © 2013 Brian Spanton

#pragma once

namespace Basic
{
    template <typename element_type>
    __interface INumberStream
    {
        bool WriteDigit(element_type digit);
        uint8 get_digit_count();
    };
}