// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"

namespace Basic
{
    template <typename element_type>
    __interface IStreamWriter
    {
        void write_to_stream(IStream<element_type>* stream) const;
    };

    template <typename element_type>
    __interface IVector
    {
        element_type* address() const;
        uint32 size() const;
    };
}