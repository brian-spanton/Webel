// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"

namespace Basic
{
    template <typename element_type>
    __interface IStreamWriter // $$ rename to something like IRenderable?  or consider another pattern
    {
        void write_to_stream(IStream<element_type>* stream) const; // $$ rename to something like render_to?  or consider another pattern
    };
}