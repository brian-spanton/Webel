// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"
#include "Basic.ElementSource.h"

namespace Basic
{
    template <typename element_type>
    __interface ITransportEventHandler
    {
        void transport_connected();
        void transport_disconnected();
        void transport_received(const element_type* elements, uint32 count);
    };
}
