// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IEvent.h"
#include "Basic.IProcess.h"
#include "Basic.IStream.h"
#include "Basic.IElementSource.h"

namespace Basic
{
    class Event
    {
    public:
        template<typename element_type>
        static void Read(IEvent* event, uint32 count, const element_type** out_address, uint32* out_count);

        template<typename element_type>
        static void ReadNext(IEvent* event, element_type* element);

        template<typename element_type>
        static void AddObserver(IEvent* event, std::shared_ptr<IStream<element_type> > stream);

        template<typename element_type>
        static void RemoveObserver(IEvent* event, std::shared_ptr<IStream<element_type> > stream);
    };
}