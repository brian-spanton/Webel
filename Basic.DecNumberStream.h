// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.INumberStream.h"

namespace Basic
{
    template <typename element_type, class number_type>
    class DecNumberStream : public INumberStream<element_type>
    {
    private:
        number_type* value;
        uint8 count;

    public:
        DecNumberStream<element_type, number_type>() :
            count(0),
            value(0)
        {
        }

        DecNumberStream<element_type, number_type>(number_type* value) :
            count(0),
            value(value)
        {
        }

        void reset()
        {
            this->count = 0;
        }

        void reset(number_type* value)
        {
            this->count = 0;
            this->value = value;
        }

        uint8 get_digit_count()
        {
            return this->count;
        }

        virtual bool INumberStream<element_type>::WriteDigit(element_type digit)
        {
            byte digit_value;

            bool success = base_10<element_type>(digit, &digit_value);
            if (!success)
                return false;

            if (this->count == 0)
            {
                (*this->value) = 0;
            }
            else
            {
                (*this->value) *= 10;
            }

            this->count += 1;
            (*this->value) += digit_value;
            return true;
        }
    };
}
