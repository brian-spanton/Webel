// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.ILog.h"

namespace Basic
{
    template <typename element_type>
    class SplitStream : public IStream<element_type>
    {
    public:
        typedef std::list<std::shared_ptr<IStream<element_type> > > OutputList;

        OutputList outputs;

        virtual void IStream<element_type>::write_element(element_type element)
        {
            for (auto output : this->outputs)
                output->write_element(element);
        }

        virtual void IStream<element_type>::write_elements(const element_type* elements, uint32 count)
        {
            for (auto output : this->outputs)
                output->write_elements(elements, count);
        }

        virtual void IStream<element_type>::write_eof()
        {
            for (auto output : this->outputs)
                output->write_eof();
        }
    };
}