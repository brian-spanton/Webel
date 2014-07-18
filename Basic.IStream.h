// Copyright © 2013 Brian Spanton

#pragma once

namespace Basic
{
    template <typename element_type>
    __interface IStream
    {
        void write_element(element_type element);
        void write_elements(const element_type* elements, uint32 count);
        void write_eof();
    };

    template <typename element_type>
    class UnitStream : public IStream<element_type>
    {
    public:
        virtual void IStream<element_type>::write_elements(const element_type* elements, uint32 count)
        {
            for (const element_type* element = elements; element != elements + count; element++)
            {
                write_element(*element);
            }
        }

        virtual void IStream<element_type>::write_eof()
        {
            HandleError("unexpected eof");
        }
    };

    template <typename element_type>
    class ArrayStream : public IStream<element_type>
    {
    public:
        virtual void IStream<element_type>::write_element(element_type element)
        {
            write_elements(&element, 1);
        }

        virtual void IStream<element_type>::write_eof()
        {
            HandleError("unexpected eof");
        }
    };
}