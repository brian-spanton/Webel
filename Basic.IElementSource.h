// Copyright © 2013 Brian Spanton

#pragma once

#include "Basic.IStream.h"

namespace Basic
{
    template <class T>
    __interface IElementSource
    {
        void Read(uint32 count, const T** out_address, uint32* out_count);
        void ReadNext(T* element);
        void AddObserver(std::shared_ptr<IStream<T> > stream);
        void RemoveObserver(std::shared_ptr<IStream<T> > stream);
        bool Exhausted();
    };
}