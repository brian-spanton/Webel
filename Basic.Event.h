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
        template<class element_type>
        static bool Read(IEvent* event, uint32 count, const element_type** out_address, uint32* out_count, bool* yield);

        template<class element_type>
        static bool ReadNext(IEvent* event, element_type* element, bool* yield);

        template<class element_type>
        static void AddObserver(IEvent* event, IStream<element_type>* stream);

        template<class element_type>
        static void RemoveObserver(IEvent* event, IStream<element_type>* stream);
    
        static void UndoReadNext(IEvent* event);
    };
}