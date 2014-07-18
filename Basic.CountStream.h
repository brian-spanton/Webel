// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"

namespace Basic
{
    template <typename element_type>
    class CountStream : public IStream<element_type>
    {
    public:
        uint32 count;

        CountStream() : 
            count(0)
        {
        }

        virtual void IStream<element_type>::write_elements(const element_type* elements, uint32 count)
        {
            this->count += count;
        }

        virtual void IStream<element_type>::write_element(element_type element)
        {
            this->count += 1;
        }

        virtual void IStream<element_type>::write_eof()
        {
            HandleError("unexpected eof");
        }
    };
}