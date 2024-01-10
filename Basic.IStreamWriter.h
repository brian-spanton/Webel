// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"

namespace Basic
{
    template <typename element_type>
    __interface IStreamWriter // $$$ rename to IRenderable
    {
        void write_to_stream(IStream<element_type>* stream) const; // $$$ rename to render_to
    };
}